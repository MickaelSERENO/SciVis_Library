#include "Datasets/VTKDataset.h"
#include <omp.h>

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

namespace sereno
{
    VTKDataset::VTKDataset(std::shared_ptr<VTKParser>& parser, const std::vector<const VTKFieldValue*>& ptFieldValues, 
                           const std::vector<const VTKFieldValue*>& cellFieldValues) : m_ptFieldValues(ptFieldValues), m_cellFieldValues(cellFieldValues), m_parser(parser)
    {
        //Create point field descriptors
        m_pointFieldDescs.resize(m_ptFieldValues.size());
        for(uint32_t i = 0; i < m_ptFieldValues.size(); i++)
        {
            const VTKFieldValue* val = m_ptFieldValues[i];
            PointFieldDesc* desc = &m_pointFieldDescs[i];
            *(static_cast<FieldValueMetaData*>(desc)) = *val;
            desc->id     = i;
            desc->minVal = INT_MIN; //Those two values are computed in the load method
            desc->maxVal = INT_MIN;
        }
    }

    VTKDataset::~VTKDataset()
    {
        if(m_readThread.joinable())
            m_readThread.join();
    }

    void VTKDataset::loadValues(LoadCallback clbk, void* data)
    {
        if(m_readThreadRunning == false)
        {
            m_readThreadRunning = true;
            m_readThread = std::thread([this, clbk, data]()
            {
                for(uint32_t i = 0; i < m_ptFieldValues.size(); i++)
                {
                    const VTKFieldValue* val = m_ptFieldValues[i];
                    uint8_t* data = (uint8_t*)m_parser->parseAllFieldValues(val);
                    m_pointFieldDescs[i].values.reset(data);

                    //Compute min/max
                    float minVal = std::numeric_limits<float>::max();
                    float maxVal = std::numeric_limits<float>::min();

                    uint8_t valueFormatInt = VTKValueFormatInt(val->format);

                    //Scalar "min/max"
                    if(m_pointFieldDescs[i].nbValuePerTuple == 1)
                    {
#if defined(_OPENMP)
                        #pragma omp parallel
#endif
                        {
#if defined(_OPENMP)
                            #pragma omp for reduction(max:maxVal) reduction(min:minVal)
#endif
                            for(uint32_t k = 0; k < val->nbTuples; k++)
                            {
                                float readVal = readParsedVTKValue<float>(data + k*valueFormatInt, val->format);
                                minVal = std::min(minVal, readVal);
                                maxVal = std::max(maxVal, readVal);
                            }
                        }
                    }

                    //Magnitude "min/max" (vector magnitude)
                    else
                    {
#if defined(_OPENMP)
                        #pragma omp parallel
#endif
                        {
#if defined(_OPENMP)
                            #pragma omp for reduction(max:maxVal) reduction(min:minVal)
#endif
                            for(uint32_t k = 0; k < val->nbTuples; k++)
                            {
                                float mag = 0;
                                for(uint32_t j = 0; j < val->nbValuePerTuple; j++)
                                {
                                    float readVal = readParsedVTKValue<float>((uint8_t*)data + k*valueFormatInt*val->nbValuePerTuple + j*valueFormatInt, val->format);
                                    mag = readVal*readVal;
                                }
                                
                                mag = sqrt(mag);
                                minVal = fmin(minVal, mag);
                                maxVal = fmax(maxVal, mag);
                            }
                        }
                    }

                    m_pointFieldDescs[i].maxVal = maxVal;
                    m_pointFieldDescs[i].minVal = minVal;
                }

                std::cout << "End thread" << std::endl;
                m_valuesLoaded = true;
                if(clbk != NULL)
                    clbk(this, 1, data);
                m_readThreadRunning = false;
            });
        }
    }

    bool VTKDataset::create1DHistogram(uint32_t* output, uint32_t width, uint32_t ptFieldXID) const
    {
        //Check property
        if(ptFieldXID >= m_pointFieldDescs.size())
        {
            std::cerr << "Point Field X could not be found." << std::endl;
            return false;
        }

        //Reinitialize the image
        for(uint32_t i=0; i < width; i++)
            output[i] = 0;

        //Constant values
        const PointFieldDesc& ptX = m_pointFieldDescs[ptFieldXID];
        const float xDiv = ptX.maxVal - ptX.minVal;
        uint8_t ptXFormat = VTKValueFormatInt(ptX.format);

#if defined(_OPENMP)
#pragma omp parallel
        {
            //Initialize a private histogram
            uint32_t* privateHisto = (uint32_t*)malloc(sizeof(uint32_t)*width);
            for(uint32_t i=0; i<width; i++) 
                privateHisto[i] = 0;

            //Check conditions OUTSIDE for loop for optimization issue
            if(ptX.nbValuePerTuple == 1)
            {
                #pragma omp for
                for(uint32_t i = 0; i < ptX.nbTuples; i++)
                {
                    float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat, ptX.format);
                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);
                    privateHisto[x]++;
                }

            } 

            else
            {
                #pragma omp for
                for(uint32_t i=0; i<ptX.nbTuples; i++) 
                {
                    float xVal = 0.0;
                    for(uint32_t k = 0; k < ptX.nbValuePerTuple; k++)
                    {
                        float val = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                        xVal = val*val;
                    }
                    xVal = sqrt(xVal);

                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);
                    privateHisto[x]++;
                }
            }

            //Merge everything
            #pragma omp critical 
            {
                for(uint32_t i=0; i<width; i++) 
                    output[i] += privateHisto[i];
            }

            free(privateHisto);
        }
#else
            //Check conditions OUTSIDE for loop for optimization issue
            if(ptX.nbValuePerTuple == 1)
            {
                for(uint32_t i = 0; i < ptX.nbTuples; i++)
                {
                    float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat, ptX.format);
                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);
                    output[x]++;
                }

            } 

            else
            {
                for(uint32_t i=0; i<ptX.nbTuples; i++) 
                {
                    float xVal = 0.0;
                    for(uint32_t k = 0; k < ptX.nbValuePerTuple; k++)
                    {
                        float val = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                        xVal = val*val;
                    }
                    xVal = sqrt(xVal);

                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);
                    output[x]++;
                }
            }
#endif

            return true;
    }

    bool VTKDataset::create2DHistogram(uint32_t* output, uint32_t width, uint32_t height, uint32_t ptFieldXID, uint32_t ptFieldYID) const
    {
        //Check property
        if(ptFieldXID >= m_pointFieldDescs.size() || ptFieldYID >= m_pointFieldDescs.size())
        {
            std::cerr << "Point Field X or Point Field Y could not be found." << std::endl;
            return false;
        }

        //Reinitialize the image
        for(uint32_t i=0; i < width*height; i++)
            output[i] = 0;

        //Fetch common values
        const PointFieldDesc& ptX = m_pointFieldDescs[ptFieldXID];
        const PointFieldDesc& ptY = m_pointFieldDescs[ptFieldYID];

        const float xDiv = ptX.maxVal - ptX.minVal;
        const float yDiv = ptY.maxVal - ptY.minVal;

        uint8_t ptXFormat = VTKValueFormatInt(ptX.format);
        uint8_t ptYFormat = VTKValueFormatInt(ptY.format);

        //Create a private histogram per thread. Work with this private histogram and merge at the end
#if defined(_OPENMP)
#pragma omp parallel
        {
            //Initialize a private histogram
            uint32_t* privateHisto = (uint32_t*)malloc(sizeof(uint32_t)*width*height);
            for(uint32_t i=0; i<width*height; i++) 
                privateHisto[i] = 0;

            //Make conditions OUTSIDE of the for loops for optimization issue
            //ptX.nbTuples == ptY.nbTuples
            if(ptX.nbValuePerTuple == 1 && ptY.nbValuePerTuple == 1)
            {
                #pragma omp for
                for(uint32_t i = 0; i < ptX.nbTuples; i++)
                {
                    float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat, ptX.format);
                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);

                    float yVal = readParsedVTKValue<float>((uint8_t*)ptY.values.get() + i*ptYFormat, ptY.format);
                    uint32_t y = MIN(height*(yVal-ptY.minVal)/yDiv, height-1);

                    privateHisto[y*width + x]++;
                }
            }

            else if(ptX.nbValuePerTuple == 1 && ptY.nbTuples > 1)
            {
                #pragma omp for
                for(uint32_t i = 0; i < ptX.nbValuePerTuple; i++)
                {
                    float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat, ptX.format);
                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);

                    //Take the magnitude
                    float yVal = 0.0;
                    for(uint32_t k = 0; k < ptY.nbValuePerTuple; k++)
                    {
                        float val = readParsedVTKValue<float>((uint8_t*)ptY.values.get() + i*ptYFormat*ptY.nbValuePerTuple + k*ptYFormat, ptY.format);
                        yVal = val*val;
                    }
                    uint32_t y = MIN(height*(sqrt(yVal) - ptY.minVal)/yDiv, height-1);

                    privateHisto[y*width + x]++;
                }
            }

            else if(ptX.nbValuePerTuple > 1 && ptY.nbValuePerTuple == 1)
            {
                #pragma omp for
                for(uint32_t i=0; i<ptX.nbTuples; i++) 
                {
                    float xVal = 0.0;
                    for(uint32_t k = 0; k < ptX.nbValuePerTuple; k++)
                    {
                        float val = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                        xVal = val*val;
                    }
                    xVal = sqrt(xVal);

                    float yVal = readParsedVTKValue<float>((uint8_t*)ptY.values.get() + i*ptYFormat, ptY.format);
                    uint32_t x = MIN((xVal - ptX.minVal)/xDiv, width-1);
                    uint32_t y = MIN((yVal - ptY.minVal)/yDiv, height-1);
                    privateHisto[y*width + x]++;
                }
            }

            else
            {
                #pragma omp for
                for(uint32_t i=0; i<ptX.nbTuples; i++) 
                {
                    float xVal = 0.0;
                    for(uint32_t k = 0; k < ptX.nbValuePerTuple; k++)
                    {
                        float val = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                        xVal = val*val;
                    }
                    xVal = sqrt(xVal);

                    float yVal = 0.0;
                    for(uint32_t k = 0; k < ptY.nbValuePerTuple; k++)
                    {
                        float val = readParsedVTKValue<float>((uint8_t*)ptY.values.get() + i*ptYFormat*ptY.nbValuePerTuple + k*ptYFormat, ptY.format);
                        yVal = val*val;
                    }
                    yVal = sqrt(yVal);

                    uint32_t x = MIN((xVal - ptX.minVal)/xDiv, width-1);
                    uint32_t y = MIN((yVal - ptY.minVal)/yDiv, height-1);
                    privateHisto[y*width + x]++;
                }
            }

            //Merge everything
            #pragma omp critical 
            {
                for(uint32_t i=0; i<width*height; i++) 
                    output[i] += privateHisto[i];
            }

            free(privateHisto);
        }
#else
        //Make conditions OUTSIDE of the for loops
        if(ptX.nbValuePerTuple == 1 && ptY.nbValuePerTuple == 1)
        {
            for(uint32_t i = 0; i < ptX.nbTuples; i++)
            {
                float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat, ptX.format);
                uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);

                float yVal = readParsedVTKValue<float>((uint8_t*)ptY.values.get() + i*ptYFormat, ptY.format);
                uint32_t y = MIN(height*(yVal-ptY.minVal)/YDiv, height-1);

                output[y*width + x]++;
            }
        }

        else if(ptX.nbValuePerTuple == 1 && ptY.nbTuples > 1)
        {
            for(uint32_t i = 0; i < ptX.nbValuePerTuple; i++)
            {
                float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat, ptX.format);
                uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);

                //Take the magnitude
                float yVal = 0.0;
                for(uint32_t k = 0; k < ptY.nbValuePerTuple; k++)
                {
                    float val = readParsedVTKValue<float>((uint8_t*)ptY.values.get() + i*ptYFormat*ptY.nbValuePerTuple + k*ptYFormat, ptY.format);
                    yVal = val*val;
                }
                uint32_t y = MIN(height*(sqrt(yVal) - ptY.minVal)/yDiv, height-1);

                output[y*width + x]++;
            }
        }

        else if(ptX.nbValuePerTuple > 1 && ptY.nbValuePerTuple == 1)
        {
            for(uint32_t i=0; i<ptX.nbTuples; i++) 
            {
                float xVal = 0.0;
                for(uint32_t k = 0; k < ptX.nbValuePerTuple; k++)
                {
                    float val = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                    xVal = val*val;
                }
                xVal = sqrt(xVal);

                float yVal = readParsedVTKValue<float>((uint8_t*)ptY.values.get() + i*ptYFormat, ptY.format);
                uint32_t x = MIN((xVal - ptX.minVal)/xDiv, width-1);
                uint32_t y = MIN((yVal - ptY.minVal)/yDiv, height-1);
                output[y*width + x]++;
            }
        }

        else
        {
            for(uint32_t i=0; i<ptX.nbTuples; i++) 
            {
                float xVal = 0.0;
                for(uint32_t k = 0; k < ptX.nbValuePerTuple; k++)
                {
                    float val = readParsedVTKValue<float>((uint8_t*)ptX.values.get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                    xVal = val*val;
                }
                xVal = sqrt(xVal);

                float yVal = 0.0;
                for(uint32_t k = 0; k < ptY.nbValuePerTuple; k++)
                {
                    float val = readParsedVTKValue<float>((uint8_t*)ptY.values.get() + i*ptYFormat*ptY.nbValuePerTuple + k*ptYFormat, ptY.format);
                    yVal = val*val;
                }
                yVal = sqrt(yVal);

                uint32_t x = MIN((xVal - ptX.minVal)/xDiv, width-1);
                uint32_t y = MIN((yVal - ptY.minVal)/yDiv, height-1);
                output[y*width + x]++;
            }
        }
#endif
        return true;
    }
}
