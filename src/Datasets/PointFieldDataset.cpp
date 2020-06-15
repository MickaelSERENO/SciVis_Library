#include "Datasets/PointFieldDataset.h"
#include "sciVisUtils.h"
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <omp.h>
#include <memory>
#include <vector>


#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

using namespace serenoSciVis;

namespace sereno
{
    PointFieldDataset::PointFieldDataset(const std::string& path) : m_filePath(path)
    {
        m_pointFieldDescs.resize(1);
        m_pointFieldDescs[0].id = 0;
        m_pointFieldDescs[0].minVal = m_pointFieldDescs[0].maxVal = 0.0f;
    }

    PointFieldDataset::~PointFieldDataset()
    {
        if(m_positions)
            free(m_positions);
        if(m_readThread.joinable())
            m_readThread.join();
    }

    void PointFieldDataset::loadValues(LoadCallback clbk, void* userData)
    {
#define _BUFFER_SIZE 4096
        if(m_readThreadRunning == false)
        {
            m_readThreadRunning = true;
            m_readThread = std::thread([this, clbk, userData]()
            {
                FILE* file = fopen(m_filePath.c_str(), "r");
                if(file != NULL)
                {
                    uint8_t buffer[512];
                    float* data = NULL;
                    float minVal = std::numeric_limits<float>::max();
                    float maxVal = -minVal;
                    uint32_t i = 0;

                    //Determine the file size
                    fseek(file, 0, SEEK_END);
                    uint32_t fileSize = ftell(file);
                    if(fileSize < 4 || fileSize%4 != 0)
                    {
                        ERROR << "The file " << m_filePath << " has an incorrect format (wrong size). Abort\n";
                        goto error;
                    }
                    fseek(file, 0, SEEK_SET);

                    //Read the number of points
                    fread(buffer, sizeof(uint8_t), sizeof(uint32_t), file);
                    m_nbPoints = uint8ToUint32(buffer);
                    if(fileSize/4 != m_nbPoints)
                    {
                        ERROR << "The file " << m_filePath << " should contain " << m_nbPoints << " data point. Contain actually " << fileSize/4 << " data point. Abort\n";
                        goto error;
                    }

                    m_positions = (float*)malloc(3*sizeof(float)*m_nbPoints);
                    data        = (float*)malloc(sizeof(float)*m_nbPoints);

                    //Read all points
                    i = m_nbPoints-1;
                    while(i > 0)
                    {
                        //Use as much as possible the whole buffer to avoid "small read chunk"
                        uint32_t nbPointInRead = MIN(i, sizeof(buffer)/(3*sizeof(float)));
                        fread(buffer, sizeof(float), 3*nbPointInRead, file);

                        //Save the data position
                        for(uint32_t j = 0; j < nbPointInRead; j++)
                        {
                            for(uint32_t k = 0; k < 3; k++)
                                m_positions[3*(m_nbPoints-1-i) + k] = uint8ToFloat(buffer+sizeof(float)*3*j+k);
                            i--;
                        }
                    }

                    //Do the same for point data (float scalar)
                    i = m_nbPoints-1;
                    while(i > 0)
                    {
                        //Use as much as possible the whole buffer to avoid "small read chunk"
                        uint32_t nbPointInRead = MIN(i, sizeof(buffer)/(sizeof(float)));
                        fread(buffer, sizeof(float), nbPointInRead, file);

                        //Save the data position
                        for(uint32_t j = 0; j < nbPointInRead; j++)
                        {
                            data[m_nbPoints-1-i] = uint8ToFloat(buffer+sizeof(float)*j);
                            minVal = MIN(minVal, data[m_nbPoints-1-i]);
                            maxVal = MAX(maxVal, data[m_nbPoints-1-i]);
                            i--;
                        }
                    }

                    m_pointFieldDescs[0].minVal = minVal;
                    m_pointFieldDescs[0].maxVal = maxVal;
                    m_pointFieldDescs[0].values.reset(data);

                    fclose(file);
                    goto endError;
error:
                    if(data != NULL)
                        free(data);

                    m_nbPoints = 0;
                    fclose(file);
                    clbk(this, 0, userData);
                    return;                    
endError:
                    clbk(this, 1, userData);
                }
                else
                {
                    clbk(this, 0, userData);
                    return;
                }
            });
        }
#undef _BUFFER_SIZE


    }

    bool PointFieldDataset::create1DHistogram(uint32_t* output, uint32_t width, uint32_t ptFieldXID) const
    {
        //Check property
        if(ptFieldXID != 0)
        {
            std::cerr << "Point Field X could not be found. Expected value: 0" << std::endl;
            return false;
        }

        //Reinitialize the image
        for(uint32_t i=0; i < width; i++)
            output[i] = 0;

        //Constant values
        const PointFieldDesc& ptX = m_pointFieldDescs[0];
        const float xDiv = ptX.maxVal - ptX.minVal;

        float* data = (float*)ptX.values.get();

#if defined(_OPENMP)
#pragma omp parallel
        {
            //Initialize a private histogram
            uint32_t* privateHisto = (uint32_t*)malloc(sizeof(uint32_t)*width);
            for(uint32_t i=0; i<width; i++) 
                privateHisto[i] = 0;

            #pragma omp for
            for(uint32_t i = 0; i < ptX.nbTuples; i++)
            {
                uint32_t x = MIN(width*(data[i]-ptX.minVal)/xDiv, width-1);
                privateHisto[x]++;
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
        {
            for(uint32_t i = 0; i < ptX.nbTuples; i++)
            {
                float xVal = data[i];
                uint32_t x = MIN(width*(xVal-ptX.minVal)/xDiv, width-1);
                output[x]++;
            }
        }
#endif

            return true;
    }

    bool PointFieldDataset::create2DHistogram(uint32_t* output, uint32_t width, uint32_t height, uint32_t ptFieldXID, uint32_t ptFieldYID) const
    {
        ERROR << "Cannot compute 2D Histogram since this kind of Dataset possess only one scalar value per data point\n";
        return false; /*!< No 2D information*/
    }
}
