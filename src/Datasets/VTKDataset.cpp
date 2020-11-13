#include "Datasets/VTKDataset.h"
#include <omp.h>

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

namespace sereno
{
    /**
     * \brief  Read a VTK Vector magnitude normalized between 0.0 and 1.0
     *
     * \param vals the raw values
     * \param ptFieldValue the point field descriptor (format, nbValuePerTuple).
     * \param x the indice to look at
     *
     * \return  the normalized magnitude 
     */
    static float readVTKValueMagnitude(uint8_t* vals, const PointFieldDesc& ptFieldValue, uint32_t x)
    {
        int formatSize = VTKValueFormatInt(ptFieldValue.format);

        float mag = 0;                                  
        for(uint32_t j = 0; j < ptFieldValue.nbValuePerTuple; j++) 
        { 
            float readVal = readParsedVTKValue<float>(vals + x*formatSize*ptFieldValue.nbValuePerTuple + j*formatSize, ptFieldValue.format); 
            mag = readVal*readVal; 
        } 
          
        mag = sqrt(mag);

        return (mag - ptFieldValue.minVal)/(ptFieldValue.maxVal - ptFieldValue.minVal);
    }

    VTKDataset::VTKDataset(std::shared_ptr<VTKParser>& parser, const std::vector<const VTKFieldValue*>& ptFieldValues, 
                           const std::vector<const VTKFieldValue*>& cellFieldValues)
    {
        m_timesteps.push_back({parser, ptFieldValues, cellFieldValues});

        //Create point field descriptors
        m_pointFieldDescs.resize(ptFieldValues.size());
        for(uint32_t i = 0; i < ptFieldValues.size(); i++)
        {
            const VTKFieldValue* val = ptFieldValues[i];
            PointFieldDesc* desc = &m_pointFieldDescs[i];
            *(static_cast<FieldValueMetaData*>(desc)) = *val;
            desc->id     = i;
            desc->minVal = std::numeric_limits<float>::max(); //Those two values are computed in the load method
            desc->maxVal = std::numeric_limits<float>::min();
        }
        
        m_minPos = glm::vec3(-0.5f, -0.5f, -0.5f);
        m_maxPos = glm::vec3(0.5f, 0.5f, 0.5f);
    }

    VTKDataset::~VTKDataset()
    {
        if(m_readThread.joinable())
            m_readThread.join();
        if(m_mask)
            free(m_mask);
    }

    bool VTKDataset::addTimestep(std::shared_ptr<VTKParser>& parser)
    {
        //Check type of dataset
        if(parser->getDatasetType() != getParser()->getDatasetType())
            return false;
        if(parser->getDatasetType() == VTK_STRUCTURED_POINTS)
            if(getParser()->getStructuredPointsDescriptor() != parser->getStructuredPointsDescriptor())
                return false;

        //Build the field values. We should find the same in the first VTKParser and in the one that is being added. If not --> error
        std::vector<const VTKFieldValue*> ptFieldValues;
        std::vector<const VTKFieldValue*> cellFieldValues;

        std::vector<const VTKFieldValue*> parserPt   = parser->getPointFieldValueDescriptors();
        std::vector<const VTKFieldValue*> parserCell = parser->getCellFieldValueDescriptors();
        for(const FieldValueMetaData* f : getPtFieldValues())
        {
            auto it = std::find_if(parserPt.begin(), parserPt.end(), [f](const FieldValueMetaData* v)
            {
                return *v == *f;
            });
            if(it == parserPt.end())
                return false;
            ptFieldValues.push_back(*it);
        }

        for(const FieldValueMetaData* f : getCellFieldValues())
        {
            auto it2 = std::find_if(parserCell.begin(), parserCell.end(), [f](const FieldValueMetaData* v){return *v == *f;});
            if(it2 == parserCell.end())
                return false;
            parserCell.push_back(*it2);
        }

        //Add the correctely parsed timestep
        m_timesteps.push_back({parser, ptFieldValues, cellFieldValues});
        m_nbTimesteps++;
        return true;
    }

    std::thread* VTKDataset::loadValues(LoadCallback clbk, void* data)
    {
        if(m_readThreadRunning == false)
        {
            m_readThreadRunning = true;
            m_readThread = std::thread([this, clbk, data]()
            {
                //Get the mask
                for(const VTKFieldValue* val : getParser()->getPointFieldValueDescriptors())
                {
                    //Search with the name
                    if(val->name == "vtkValidPointMask" && val->nbValuePerTuple == 1)
                    {
                        //Save some space by using 1 bit == 1 value
                        uint8_t* maskData = (uint8_t*)getParser()->parseAllFieldValues(val);
                        m_mask = (uint8_t*)calloc((val->nbTuples+7)/8, sizeof(uint8_t));
                        for(uint32_t i = 0, k=0; i < val->nbTuples; k++)
                            for(uint32_t j = 0; j < 8 && i < val->nbTuples; j++, i++)
                                if(maskData[i]) 
                                    m_mask[k] |= (1 << j);

                        free(maskData);
                        break;
                    }
                }

                for(uint32_t t = 0; t < m_timesteps.size(); t++)
                {
                    for(uint32_t i = 0; i < getPtFieldValues().size(); i++)
                    {
                        const VTKTimestep& timestepData = getTimestep(t);
                        const VTKFieldValue* val = timestepData.ptFieldValues[i];
                        uint8_t* data = (uint8_t*)timestepData.parser->parseAllFieldValues(val);

                        //Compute min/max
                        double minVal = m_pointFieldDescs[i].minVal;
                        double maxVal = m_pointFieldDescs[i].maxVal;

                        uint8_t valueFormatInt = VTKValueFormatInt(val->format);


                        //Scalar "min/max"
                        if(m_pointFieldDescs[i].nbValuePerTuple == 1)
                        {
#if defined(_OPENMP)
                            #pragma omp parallel for reduction(max:maxVal) reduction(min:minVal)
#endif
                            for(uint32_t k = 0; k < val->nbTuples; k++)
                            {
                                if(getMask(k))
                                {
                                    double readVal = readParsedVTKValue<double>(data + k*valueFormatInt, val->format);
                                    minVal = (minVal < readVal ? minVal : readVal);
                                    maxVal = (maxVal > readVal ? maxVal : readVal);
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
                                    if(!getMask(k))
                                        continue;
                                    double mag = 0;
                                    for(uint32_t j = 0; j < val->nbValuePerTuple; j++)
                                    {
                                        double readVal = readParsedVTKValue<double>(data + k*valueFormatInt*val->nbValuePerTuple + j*valueFormatInt, val->format);
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
                        m_pointFieldDescs[i].values.emplace_back(data, _FreeDeleter());
                    }
                }

                std::vector<uint32_t> fields;
                for(uint32_t i = 0; i < getPtFieldValues().size(); i++)
                    fields.push_back(i);
                getOrComputeGradient(fields);

                //Computation done, set the state to "loaded" and call the callback function
                m_valuesLoaded = true;
                if(clbk != NULL)
                    clbk(this, 1, data);
                m_readThreadRunning = false;
            });

            return &m_readThread;
        }

        return NULL;
    }

    DatasetGradient* VTKDataset::computeGradient(const std::vector<uint32_t>& indices)
    {
        const VTKStructuredPoints& ptsDesc = getParser()->getStructuredPointsDescriptor();

        /*----------------------------------------------------------------------------*/
        /*--------------------------Compute gradient values---------------------------*/
        /*----------------------------------------------------------------------------*/
        DatasetGradient* gradientData = new DatasetGradient();
        gradientData->indices = indices;

        float maxGrad=0;

        for(uint32_t t = 0; t < m_nbTimesteps; t++)
        {
            float* grads = (float*)malloc(sizeof(float)*ptsDesc.size[0]*ptsDesc.size[1]*ptsDesc.size[2]);
            if(indices.size() > 1)
            {
                //The Df matrice, see the doxygen
#ifdef _OPENMP
                #pragma omp parallel reduction(max:maxGrad)
#endif
                {
                    float* df = (float*)malloc(sizeof(float)*3*indices.size());

#ifdef _OPENMP
                    #pragma omp for
#endif
                    for(uint32_t k = 1; k < ptsDesc.size[2]-1; k++)
                        for(uint32_t j = 1; j < ptsDesc.size[1]-1; j++)
                            for(uint32_t i = 1; i < ptsDesc.size[0]-1; i++)
                            {
                                uint32_t ind = i + j*ptsDesc.size[0] + k*ptsDesc.size[0]*ptsDesc.size[1];

                                if(!getMask(ind))
                                {
                                    grads[ind] = 0;
                                    continue;
                                }
                                for(uint32_t l = 0; l < indices.size(); l++)
                                {
                                    const PointFieldDesc& ptFieldValue = m_pointFieldDescs[indices[l]];
                                    int formatSize = VTKValueFormatInt(ptFieldValue.format);
                                    uint8_t* vals = (uint8_t*)ptFieldValue.values[t].get();

                                    if(ptFieldValue.nbValuePerTuple == 1)
                                    {
                                        float    x1  = readParsedVTKValue<float>(vals + (ind-1)*formatSize, ptFieldValue.format);
                                        float    x2  = readParsedVTKValue<float>(vals + (ind+1)*formatSize, ptFieldValue.format);
                                        float    y1  = readParsedVTKValue<float>(vals + (ind-ptsDesc.size[0])*formatSize, ptFieldValue.format);
                                        float    y2  = readParsedVTKValue<float>(vals + (ind+ptsDesc.size[0])*formatSize, ptFieldValue.format);
                                        float    z1  = readParsedVTKValue<float>(vals + (ind-ptsDesc.size[0]*ptsDesc.size[1])*formatSize, ptFieldValue.format);
                                        float    z2  = readParsedVTKValue<float>(vals + (ind+ptsDesc.size[0]*ptsDesc.size[1])*formatSize, ptFieldValue.format);

                                        float gradX = (x2-x1)/(2.0f*ptsDesc.spacing[0]);
                                        float gradY = (y2-y1)/(2.0f*ptsDesc.spacing[1]);
                                        float gradZ = (z2-z1)/(2.0f*ptsDesc.spacing[2]);
                                        
                                        df[3*l+0] = gradX/(ptFieldValue.maxVal-ptFieldValue.minVal);
                                        df[3*l+1] = gradY/(ptFieldValue.maxVal-ptFieldValue.minVal);
                                        df[3*l+2] = gradZ/(ptFieldValue.maxVal-ptFieldValue.minVal);
                                    }

                                    else
                                    {
                                        float    x1  = readVTKValueMagnitude(vals, ptFieldValue, ind-1);
                                        float    x2  = readVTKValueMagnitude(vals, ptFieldValue, ind+1);
                                        float    y1  = readVTKValueMagnitude(vals, ptFieldValue, ind-ptsDesc.size[0]);
                                        float    y2  = readVTKValueMagnitude(vals, ptFieldValue, ind+ptsDesc.size[0]);
                                        float    z1  = readVTKValueMagnitude(vals, ptFieldValue, ind-ptsDesc.size[0]*ptsDesc.size[1]);
                                        float    z2  = readVTKValueMagnitude(vals, ptFieldValue, ind+ptsDesc.size[0]*ptsDesc.size[1]);

                                        float gradX = (x2-x1)/(2.0f*ptsDesc.spacing[0]);
                                        float gradY = (y2-y1)/(2.0f*ptsDesc.spacing[1]);
                                        float gradZ = (z2-z1)/(2.0f*ptsDesc.spacing[2]);

                                        df[3*l+0] = gradX;
                                        df[3*l+1] = gradY;
                                        df[3*l+2] = gradZ;
                                    }
                                }

                                float g[9]; //(Df)^T . Df
                                for(uint32_t l = 0; l < 9; l++)
                                    g[l] = 0.0f;
                                
                                for(uint32_t n = 0; n < indices.size(); n++)
                                    for(uint32_t l = 0; l < 3; l++)
                                        for(uint32_t m = 0; m < 3; m++)
                                            g[3*l+m] += df[3*n+l]*df[3*n+m];

                                float gradMag = 0; //L2 norm
                                for(uint32_t l = 0; l < 9; l++)
                                    gradMag += g[l]*g[l];

                                gradMag = sqrt(gradMag);
                                grads[ind] = gradMag;

                                maxGrad = std::max(gradMag, maxGrad);
                            }

                    free(df);
                }
            }
            else if(indices.size() == 1)
            {
                const PointFieldDesc& ptFieldValue = m_pointFieldDescs[indices[0]];
                int formatSize = VTKValueFormatInt(ptFieldValue.format);
                uint8_t* vals = (uint8_t*)ptFieldValue.values[t].get();

                if(ptFieldValue.nbValuePerTuple == 1)
                {
#ifdef _OPENMP
                    #pragma omp parallel for reduction(max:maxGrad)
#endif
                    for(uint32_t k = 1; k < ptsDesc.size[2]-1; k++)
                        for(uint32_t j = 1; j < ptsDesc.size[1]-1; j++)
                            for(uint32_t i = 1; i < ptsDesc.size[0]-1; i++)
                            {
                                uint32_t ind = i + j*ptsDesc.size[0] + k*ptsDesc.size[0]*ptsDesc.size[1];
                                if(!getMask(ind))
                                {
                                    grads[ind] = 0;
                                    continue;
                                }

                                float    x1  = readParsedVTKValue<float>(vals + (ind-1)*formatSize, ptFieldValue.format);
                                float    x2  = readParsedVTKValue<float>(vals + (ind+1)*formatSize, ptFieldValue.format);
                                float    y1  = readParsedVTKValue<float>(vals + (ind-ptsDesc.size[0])*formatSize, ptFieldValue.format);
                                float    y2  = readParsedVTKValue<float>(vals + (ind+ptsDesc.size[0])*formatSize, ptFieldValue.format);
                                float    z1  = readParsedVTKValue<float>(vals + (ind-ptsDesc.size[0]*ptsDesc.size[1])*formatSize, ptFieldValue.format);
                                float    z2  = readParsedVTKValue<float>(vals + (ind+ptsDesc.size[0]*ptsDesc.size[1])*formatSize, ptFieldValue.format);

                                float gradX = (x2-x1)/(2.0f*ptsDesc.spacing[0]);
                                float gradY = (y2-y1)/(2.0f*ptsDesc.spacing[1]);
                                float gradZ = (z2-z1)/(2.0f*ptsDesc.spacing[2]);

                                gradX = gradX/(ptFieldValue.maxVal-ptFieldValue.minVal);
                                gradY = gradY/(ptFieldValue.maxVal-ptFieldValue.minVal);
                                gradZ = gradZ/(ptFieldValue.maxVal-ptFieldValue.minVal);

                                float gradMag = gradX*gradX + gradY*gradY + gradZ*gradZ;
                                gradMag = sqrt(gradMag);
                                grads[ind] = gradMag;
                                maxGrad = std::max(gradMag, maxGrad);
                            }
                }

                else
                {
#ifdef _OPENMP
                    #pragma omp parallel for reduction(max:maxGrad)
#endif
                    for(uint32_t k = 1; k < ptsDesc.size[2]-1; k++)
                        for(uint32_t j = 1; j < ptsDesc.size[1]-1; j++)
                            for(uint32_t i = 1; i < ptsDesc.size[0]-1; i++)
                            {
                                uint32_t ind = i + j*ptsDesc.size[0] + k*ptsDesc.size[0]*ptsDesc.size[1];
                                if(!getMask(ind))
                                {
                                    grads[ind] = 0;
                                    continue;
                                }

                                float    x1  = readVTKValueMagnitude(vals, ptFieldValue, ind-1);
                                float    x2  = readVTKValueMagnitude(vals, ptFieldValue, ind+1);
                                float    y1  = readVTKValueMagnitude(vals, ptFieldValue, ind-ptsDesc.size[0]);
                                float    y2  = readVTKValueMagnitude(vals, ptFieldValue, ind+ptsDesc.size[0]);
                                float    z1  = readVTKValueMagnitude(vals, ptFieldValue, ind-ptsDesc.size[0]*ptsDesc.size[1]);
                                float    z2  = readVTKValueMagnitude(vals, ptFieldValue, ind+ptsDesc.size[0]*ptsDesc.size[1]);

                                float gradX = (x2-x1)/(2.0f*ptsDesc.spacing[0]);
                                float gradY = (y2-y1)/(2.0f*ptsDesc.spacing[1]);
                                float gradZ = (z2-z1)/(2.0f*ptsDesc.spacing[2]);

                                float gradMag = gradX*gradX + gradY*gradY + gradZ*gradZ;
                                gradMag = sqrt(gradMag);
                                grads[ind] = gradMag;
                                maxGrad = std::max(gradMag, maxGrad);
                            }
                }
            }

            /*----------------------------------------------------------------------------*/
            /*---------------Compute gradient values for Edge (grad = 0.0f)---------------*/
            /*----------------------------------------------------------------------------*/

#ifdef _OPENMP
            #pragma omp parallel
#endif
            {
                //for k = 0 and k = max
#ifdef _OPENMP
                #pragma omp for
#endif
                for(uint32_t j = 0; j < ptsDesc.size[1]; j++)
                    for(uint32_t i = 0; i < ptsDesc.size[0]; i++)
                    {
                        uint32_t offset = (ptsDesc.size[2]-1)*ptsDesc.size[0]*ptsDesc.size[1];
                        grads[(i+j*ptsDesc.size[0])]        = 0.0f;
                        grads[(i+j*ptsDesc.size[0]+offset)] = 0.0f;
                    }

                //for j = 0 and j = max
#ifdef _OPENMP
                #pragma omp for
#endif
                for(uint32_t k = 0; k < ptsDesc.size[2]; k++)
                    for(uint32_t i = 0; i < ptsDesc.size[0]; i++)
                    {
                        uint32_t offset = (ptsDesc.size[1]-1)*ptsDesc.size[0];
                        grads[(i+k*ptsDesc.size[0]*ptsDesc.size[1])]        = 0.0f;
                        grads[(i+k*ptsDesc.size[0]*ptsDesc.size[1]+offset)] = 0.0f;
                    }

                //for i = 0 and i = max
#ifdef _OPENMP
                #pragma omp for
#endif
                for(uint32_t k = 0; k < ptsDesc.size[2]; k++)
                    for(uint32_t j = 0; j < ptsDesc.size[1]; j++)
                    {
                        uint32_t offset = ptsDesc.size[0]-1;
                        grads[(j*ptsDesc.size[0]+
                                k*ptsDesc.size[0]*ptsDesc.size[1])]        = 0.0f;
                        grads[(j*ptsDesc.size[0]+
                                k*ptsDesc.size[0]*ptsDesc.size[1]+offset)] = 0.0f;
                    }
            }

            gradientData->grads.emplace_back(grads, _FreeDeleter());
        }

        gradientData->maxVal = maxGrad;
        return gradientData;
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

        for(uint32_t t = 0; t < ptX.values.size(); t++)
        {
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
                        float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat, ptX.format);
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
                            float val = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
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
                    float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat, ptX.format);
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
                        float val = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                        xVal = val*val;
                    }
                    xVal = sqrt(xVal);

                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);
                    output[x]++;
                }
            }
#endif
        }
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
        memset(output, 0x00, sizeof(uint32_t)*width*height);

        //Fetch common values
        const PointFieldDesc& ptX = m_pointFieldDescs[ptFieldXID];
        const PointFieldDesc& ptY = m_pointFieldDescs[ptFieldYID];

        const float xDiv = ptX.maxVal - ptX.minVal;
        const float yDiv = ptY.maxVal - ptY.minVal;

        const int ptXFormat = VTKValueFormatInt(ptX.format);
        const int ptYFormat = VTKValueFormatInt(ptY.format);

        for(uint32_t t = 0; t < ptX.values.size() && t < ptY.values.size(); t++)
        {
            //Create a private histogram per thread. Work with this private histogram and merge at the end
#ifdef _OPENMP
#pragma omp parallel
            {
                //Initialize a private histogram
                uint32_t* privateHisto = (uint32_t*)malloc(sizeof(uint32_t)*width*height);
                memset(privateHisto, 0x00, sizeof(uint32_t)*width*height);

                //Make conditions OUTSIDE of the for loops for optimization issue
                //ptX.nbTuples == ptY.nbTuples
                if(ptX.nbValuePerTuple == 1 && ptY.nbValuePerTuple == 1)
                {
                    #pragma omp for
                    for(uint32_t i = 0; i < ptX.nbTuples; i++)
                    {
                        float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat, ptX.format);
                        uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);

                        float yVal = readParsedVTKValue<float>((uint8_t*)ptY.values[t].get() + i*ptYFormat, ptY.format);
                        uint32_t y = MIN(height*(yVal-ptY.minVal)/yDiv, height-1);

                        privateHisto[y*width + x]+=1;
                    }
                }

                else if(ptX.nbValuePerTuple == 1 && ptY.nbTuples > 1)
                {
                    #pragma omp for
                    for(uint32_t i = 0; i < ptX.nbValuePerTuple; i++)
                    {
                        float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat, ptX.format);
                        uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);

                        //Take the magnitude
                        float yVal = 0.0;
                        for(uint32_t k = 0; k < ptY.nbValuePerTuple; k++)
                        {
                            float val = readParsedVTKValue<float>((uint8_t*)ptY.values[t].get() + i*ptYFormat*ptY.nbValuePerTuple + k*ptYFormat, ptY.format);
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
                            float val = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                            xVal = val*val;
                        }
                        xVal = sqrt(xVal);

                        float yVal = readParsedVTKValue<float>((uint8_t*)ptY.values[t].get() + i*ptYFormat, ptY.format);
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
                            float val = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                            xVal = val*val;
                        }
                        xVal = sqrt(xVal);

                        float yVal = 0.0;
                        for(uint32_t k = 0; k < ptY.nbValuePerTuple; k++)
                        {
                            float val = readParsedVTKValue<float>((uint8_t*)ptY.values[t].get() + i*ptYFormat*ptY.nbValuePerTuple + k*ptYFormat, ptY.format);
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
                    float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat, ptX.format);
                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);

                    float yVal = readParsedVTKValue<float>((uint8_t*)ptY.values[t].get() + i*ptYFormat, ptY.format);
                    uint32_t y = MIN(height*(yVal-ptY.minVal)/yDiv, height-1);

                    output[y*width + x]++;
                }
            }

            else if(ptX.nbValuePerTuple == 1 && ptY.nbTuples > 1)
            {
                for(uint32_t i = 0; i < ptX.nbValuePerTuple; i++)
                {
                    float xVal = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat, ptX.format);
                    uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);

                    //Take the magnitude
                    float yVal = 0.0;
                    for(uint32_t k = 0; k < ptY.nbValuePerTuple; k++)
                    {
                        float val = readParsedVTKValue<float>((uint8_t*)ptY.values[t].get() + i*ptYFormat*ptY.nbValuePerTuple + k*ptYFormat, ptY.format);
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
                        float val = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                        xVal = val*val;
                    }
                    xVal = sqrt(xVal);

                    float yVal = readParsedVTKValue<float>((uint8_t*)ptY.values[t].get() + i*ptYFormat, ptY.format);
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
                        float val = readParsedVTKValue<float>((uint8_t*)ptX.values[t].get() + i*ptXFormat*ptX.nbValuePerTuple + k*ptXFormat, ptX.format);
                        xVal = val*val;
                    }
                    xVal = sqrt(xVal);

                    float yVal = 0.0;
                    for(uint32_t k = 0; k < ptY.nbValuePerTuple; k++)
                    {
                        float val = readParsedVTKValue<float>((uint8_t*)ptY.values[t].get() + i*ptYFormat*ptY.nbValuePerTuple + k*ptYFormat, ptY.format);
                        yVal = val*val;
                    }
                    yVal = sqrt(yVal);

                    uint32_t x = MIN((xVal - ptX.minVal)/xDiv, width-1);
                    uint32_t y = MIN((yVal - ptY.minVal)/yDiv, height-1);
                    output[y*width + x]++;
                }
            }
#endif
        }
        return true;
    }
}
