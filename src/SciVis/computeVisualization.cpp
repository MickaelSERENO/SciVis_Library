#include "SciVis/computeVisualization.h"
#include "Datasets/VTKDataset.h"
#include "SciVisColor.h"
#include "writeData.h"
#include <filesystem>
#include <algorithm>
#include <omp.h>
#include <limits>
#include <cstdlib>

namespace sereno
{
    uint8_t* getVTKStructuredGridColorArray(SubDataset* sd, uint32_t* sizeOutput)
    {
        std::shared_ptr<TF>        tf      = sd->getTransferFunction();
        VTKDataset*                dataset = (VTKDataset*)sd->getParent();
        std::shared_ptr<VTKParser> parser  = dataset->getParser();

        if(parser->getDatasetType() != VTK_STRUCTURED_POINTS)
        {
            ERROR << "The SubDataset is not a VTK_STRUCTURED_POINTS. Returning..." << std::endl;
            return nullptr;
        }

        const VTKStructuredPoints&         ptsDesc      = parser->getStructuredPointsDescriptor();
        const std::vector<PointFieldDesc>& ptFieldDescs = dataset->getPointFieldDescs();

        if(!tf || tf->getDimension() - tf->hasGradient() > ptFieldDescs.size())
        {
            ERROR << "The SubDataset does not contain a valid Transfer function. Returning..." << std::endl;
            return nullptr;
        }

        //The RGBA data variables (nb values and array of colors)
        size_t   nbValues  = ptsDesc.size[0] * ptsDesc.size[1] * ptsDesc.size[2];
        uint8_t* cols      = (uint8_t*)malloc(sizeof(uint8_t)*nbValues*4);

        //Check for the indice enabled
        std::vector<uint32_t> indices;
        for(uint32_t h = 0; h < tf->getDimension() - tf->hasGradient(); h++)
            if(tf->getEnabledDimensions()[h])
                indices.push_back(ptFieldDescs[h].id);

        //Get the associated gradient
        DatasetGradient* grad = dataset->getOrComputeGradient(indices);

        float    t  = tf->getCurrentTimestep();
        uint32_t t1 = std::min((uint32_t)floor(t), dataset->getNbTimesteps());
        uint32_t t2 = std::min((uint32_t)ceil (t), dataset->getNbTimesteps());

        //Use the transfer function to generate the 3D texture
        #pragma omp parallel
        {
            float* tfIndT1 = (float*)malloc(2*tf->getDimension()*sizeof(float)); //The indice of the transfer function for the first timestep
            float* tfIndT2 = tfIndT1 + tf->getDimension(); //The indice of the transfer function for the second timestep. DO NOT FREE

            struct {float* tfInd; uint32_t t;} tfInds[] = {{tfIndT1, t1}, {tfIndT2, t2}};

            #pragma omp for schedule(static)
            //For all values in the grid
            for(uint32_t k = 0; k < ptsDesc.size[2]; k++)
            {
                for(uint32_t j = 0; j < ptsDesc.size[1]; j++)
                {
                    for(uint32_t i = 0; i < ptsDesc.size[0]; i++)
                    {
                        size_t destID = i+
                                        j*ptsDesc.size[0]+
                                        k*ptsDesc.size[0]*ptsDesc.size[1];

                        if(!dataset->getMask(destID) ||
                           (sd->isVolumetricMaskEnabled() && !sd->getVolumetricMaskAt(destID)))
                        {
                            for(uint8_t h = 0; h < 3; h++)
                                cols[4*destID+h] = 0;
                            cols[4*destID+3] = 0;
                            continue;
                        }

                        for(const auto& tfInd : tfInds)
                        {
                            //For each parameter (e.g., temperature, presure, etc.)
                            for(uint32_t h = 0; h < tf->getDimension() - tf->hasGradient(); h++)
                            {
                                if(tf->getEnabledDimensions()[h])
                                {
                                    const PointFieldDesc& val            = ptFieldDescs[h];
                                    uint8_t               valueFormatInt = VTKValueFormatInt(val.format);

                                    //Compute the vector magnitude
                                    float mag = 0;
                                    for(uint32_t l = 0; l < val.nbValuePerTuple; l++)
                                    {
                                        float readVal = readParsedVTKValue<float>((uint8_t*)(val.values[tfInd.t].get()) + destID*valueFormatInt*val.nbValuePerTuple + l*valueFormatInt, val.format);
                                        mag = readVal*readVal;
                                    }
                                    mag = sqrt(mag);

                                    //Save it at the correct indice in the TF indice (clamped into [0,1])
                                    tfInd.tfInd[h] = (mag-val.minVal)/(val.maxVal-val.minVal);
                                }
                                else
                                    tfInd.tfInd[h] = 0;
                            }

                            //Do not forget the gradient (clamped)!
                            if(tf->hasGradient())
                            {
                                if(grad)
                                    tfInd.tfInd[tf->getDimension()-1] = grad->grads[0].get()[destID];
                                else
                                    tfInd.tfInd[tf->getDimension()-1] = 0;
                            }
                        }

                        //Apply the transfer function
                        double intPart;
                        uint8_t outColT1[4];
                        tf->computeColor(tfIndT1, outColT1);
                        uint8_t outColT2[4];
                        tf->computeColor(tfIndT2, outColT2);
                        for(uint8_t h = 0; h < 3; h++)
                            cols[4*destID+h] = ((float)outColT1[h] * (1.0f-modf(t, &intPart)) + (float)outColT2[h] * modf(t, &intPart));
                        cols[4*destID+3] = tf->computeAlpha(tfIndT1);
                    }
                }
            }
            free(tfIndT1);
        }

        if(sizeOutput)
            for(uint8_t i = 0; i < 3; i++)
                sizeOutput[i] = ptsDesc.size[i];
        return cols;
    }

    bool createDirectories(const std::string& path)
    {
        std::filesystem::path p(path);
        if(std::filesystem::exists(p.parent_path()))
            return true; //No need to create directories that exist
        return std::filesystem::create_directories(p.parent_path());
    }

    bool saveVTKStructuredGridVisual(SubDataset* sd, const std::string& path)
    {
        if(!createDirectories(path))
        {
            ERROR << "Could not create the directories required to create the file " << path << std::endl;
            return false;
        }

        //Compute the color
        uint32_t size[3];
        size_t   totalSize = sizeof(uint32_t)*(size[0] * size[1] * size[2]);
        uint8_t* color = getVTKStructuredGridColorArray(sd, size);
        
        //Create the file
        FILE* file = fopen(path.c_str(), "wb");

        //Buffer + save size
        const size_t BUFFER_SIZE = 4096;
        uint8_t  data[BUFFER_SIZE];
        uint16_t offset = 0;
        writeUint32(data,                      size[0]);
        writeUint32(data +   sizeof(uint32_t), size[1]);
        writeUint32(data + 2*sizeof(uint32_t), size[2]);
        offset = 3*sizeof(uint32_t);

        //Store data and save in batches
        for(size_t i = 0; i < totalSize;)
        {
            for(; offset < BUFFER_SIZE && i < totalSize; offset += sizeof(uint32_t), i+= sizeof(uint32_t))
                writeUint32(data+offset, color[i]);
            fwrite(data, offset, 1, file);
            offset = 0;
        }

        fclose(file);
        return true;
    }

    bool saveCloudPointVisual(SubDataset* sd, const std::string& path)
    {
        if(!createDirectories(path))
        {
            ERROR << "Could not create the directories required to create the file " << path << std::endl;
            return false;
        }

        return true;
    }
}
