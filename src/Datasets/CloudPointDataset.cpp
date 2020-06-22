#include "Datasets/CloudPointDataset.h"
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
    /** \brief  Utilitary function permitting to read the number of points of a file
     * \param file the file to read. Its current cursor position is saved and restored 
     * \param succeed a reference set at true if the function succeed its read, at false otherwise
     * \return   the number of points the file contain. Set at 0 if an error occured (see 'succeed')
     */
    float _readNbPointsMetaData(FILE* file, bool& succeed)
    {
        size_t curPos = 0;
        succeed = false;
        float nbPoints = 0;
        uint8_t buffer[4];
        if(file != NULL)
        {
            curPos = ftell(file);
            //Determine the file size
            fseek(file, 0, SEEK_END);
            uint32_t fileSize = ftell(file);
            if(fileSize < 4 || fileSize%4 != 0)
            {
                ERROR << "The file has an incorrect format (wrong size). Abort\n";
                goto error;
            }
            fseek(file, 0, SEEK_SET);

            //Read the number of points
            fread(buffer, sizeof(uint8_t), sizeof(uint32_t), file);
            nbPoints = uint8ToUint32(buffer);
            if((fileSize-4)/(4*sizeof(float)) != nbPoints) //Escape the meta data to check the file size
            {
                ERROR << "The file should contain " << nbPoints << " data point. Contain actually " << (fileSize-4)/(4*sizeof(float)) << " data point. Abort\n";
                goto error;
            }
        }

        succeed = true;
error:
        if(file)
            fseek(file, curPos, SEEK_SET);
        return nbPoints;
    }

    CloudPointDataset::CloudPointDataset(const std::string& path) : m_filePath(path)
    {
        m_pointFieldDescs.resize(1);
        m_pointFieldDescs[0].id = 0;
        m_pointFieldDescs[0].minVal = m_pointFieldDescs[0].maxVal = 0.0f;

        //Read meta data
        FILE* file = fopen(m_filePath.c_str(), "r");
        bool succeed;
        m_nbPoints = _readNbPointsMetaData(file, succeed);
        fclose(file);
    }

    CloudPointDataset::~CloudPointDataset()
    {
        if(m_positions)
            free(m_positions);
        if(m_readThread.joinable())
            m_readThread.join();
    }

    void CloudPointDataset::loadValues(LoadCallback clbk, void* userData)
    {
        if(m_nbPoints == 0)
        {
            WARNING << "No points to read about... Does the file exist?\n";
            clbk(this, 0, userData);
        }
#define _BUFFER_SIZE 4096
        if(m_readThreadRunning == false)
        {
            m_readThreadRunning = true;
            m_readThread = std::thread([this, clbk, userData]()
            {
                //Normally, with the constructor, this should always exist. But well...
                FILE* file = fopen(m_filePath.c_str(), "r");
                if(file != NULL)
                {
                    fseek(file, 4, SEEK_SET); //Skip metadata

                    uint8_t buffer[_BUFFER_SIZE];
                    float* data = NULL;
                    float minVal = std::numeric_limits<float>::max();
                    float maxVal = -minVal;
                    int32_t i = 0;

                    m_positions = (float*)malloc(3*sizeof(float)*m_nbPoints);
                    data        = (float*)malloc(sizeof(float)*m_nbPoints);

                    //Read all points
                    i = m_nbPoints-1;
                    while(i >= 0)
                    {
                        //Use as much as possible the whole buffer to avoid "small read chunk"
                        uint32_t nbPointInRead = MIN(i+1, (int32_t)(_BUFFER_SIZE/(3*sizeof(float))));
                        fread(buffer, sizeof(uint8_t), 3*sizeof(float)*nbPointInRead, file);

                        //Save the data position
                        for(uint32_t j = 0; j < nbPointInRead; j++, i--)
                            for(uint32_t k = 0; k < 3; k++)
                                m_positions[3*(m_nbPoints-1-i) + k] = uint8ToFloat(buffer+sizeof(float)*(3*j+k));
                    }

                    //Do the same for point data (float scalar)
                    i = m_nbPoints-1;
                    while(i >= 0)
                    {
                        //Use as much as possible the whole buffer to avoid "small read chunk"
                        uint32_t nbPointInRead = MIN(i+1, (int32_t)(_BUFFER_SIZE/(sizeof(float))));
                        fread(buffer, sizeof(uint8_t), nbPointInRead*sizeof(float), file);

                        //Save the data position
                        for(uint32_t j = 0; j < nbPointInRead; j++, i--)
                        {
                            data[m_nbPoints-1-i] = uint8ToFloat(buffer+sizeof(float)*j);
                            minVal = MIN(minVal, data[m_nbPoints-1-i]);
                            maxVal = MAX(maxVal, data[m_nbPoints-1-i]);
                        }
                    }

                    m_pointFieldDescs[0].minVal = minVal;
                    m_pointFieldDescs[0].maxVal = maxVal;
                    m_pointFieldDescs[0].values.reset(data);

                    m_valuesLoaded = true;

                    fclose(file);
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

    bool CloudPointDataset::create1DHistogram(uint32_t* output, uint32_t width, uint32_t ptFieldXID) const
    {
        //Check property
        if(ptFieldXID != 0)
        {
            ERROR << "Point Field X could not be found. Expected value: 0" << std::endl;
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

    bool CloudPointDataset::create2DHistogram(uint32_t* output, uint32_t width, uint32_t height, uint32_t ptFieldXID, uint32_t ptFieldYID) const
    {
        ERROR << "Cannot compute 2D Histogram since this kind of Dataset possess only one scalar value per data point\n";
        return false; /*!< No 2D information*/
    }
}
