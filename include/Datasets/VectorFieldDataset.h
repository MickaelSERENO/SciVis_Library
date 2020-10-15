#ifndef  VECTORFIELDDATASET_INC
#define  VECTORFIELDDATASET_INC

#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits>
#include "Quaternion.h"
#include "ColorMode.h"
#include "Dataset.h"

namespace sereno
{
    /*  \brief Class representing the fluid datasets */
    class VectorFieldDataset : public Dataset
    {
        public:
            /* \brief Constructor. Read the file defined by dataPath 
             * \param file the file to read
             * \param name the VectorFieldDataset name*/
            VectorFieldDataset(FILE* file, const std::string& name);

            /* \brief Copy constructor
             * \param copy the VectorFieldDataset to copy */
            VectorFieldDataset(const VectorFieldDataset& copy);

            /* \brief movement constructor
             * \param mvt the variable to move */
            VectorFieldDataset(VectorFieldDataset&& mvt);

            /* \brief operator=
             * \param copy the VectorFieldDataset to copy */
            VectorFieldDataset& operator=(const VectorFieldDataset& copy);

            /* \brief Destructor. */
            ~VectorFieldDataset();

            /* \brief Create a VectorFieldDataset from a filepath.
             * \param path the file path to read at
             * \param the created VectorFieldDataset, null if the file is not defined 
             * \return the VectorFieldDataset. Destroy it using delete operator*/
            static VectorFieldDataset* readFromFilePath(const std::string& path);

            /* \brief Get the velocity array. Size : getGridSize
             * \return the velocity array packed in (x, y, z) like : 
             * for(int k = 0; k < getGridSize[2]; k++)
             *     for(int j = 0; j < getGridSize[1]; j++)
             *         for(int i = 0; i < getGridSize[0]; i++)
             *         {
             *             float x = m_velocity[3*(i+j*getGriSize[0]+k*getGridSize[1]*getGridSize[2])+0];
             *             float y = m_velocity[3*(i+j*getGriSize[0]+k*getGridSize[1]*getGridSize[2])+1];
             *             float z = m_velocity[3*(i+j*getGriSize[0]+k*getGridSize[1]*getGridSize[2])+2];
             *         } */
            const float* getVelocity() const {return m_velocity;}

            /* \brief Get the grid size of this dataset
             * \return the grid size as a pointer with length == 3 */
            const uint32_t* getGridSize() const {return m_size;}

            /* \brief Compute how many cells exist in this grid (X*Y*Z)
             * \return the number of cells */
            const uint32_t  nbCells() const {return m_size[0]*m_size[1]*m_size[2];}

            /* \brief Get the rotation quaternion of this cell (orientation of the vector)
             * \param x the x coordinate
             * \param y the y coordinate
             * \param z the z coordinate
             * \return the direction encoded in a Quaternion */
            Quaternionf getRotationQuaternion(uint32_t x, uint32_t y, uint32_t z) const;

            virtual std::thread* loadValues(LoadCallback clbk, void* data) {clbk(this, 0, data); return NULL;} //TODO

            virtual bool create1DHistogram(uint32_t* output, uint32_t width, uint32_t ptFieldXID) const {return false;} //TODO

            virtual bool create2DHistogram(uint32_t* output, uint32_t width, uint32_t height, uint32_t ptFieldXID, uint32_t ptFieldYID) const
            {
                std::cerr << "Histograms cannot be computed for this type of Dataset because it contains only one fieldID" << std::endl;
                return false;
            }

        protected:
            virtual DatasetGradient* computeGradient(const std::vector<uint32_t>& indices) {return NULL;}

        private:
            uint32_t m_size[3];           /*!< The 3D size of the grid*/
            float*   m_velocity = NULL;   /*!< The velocity array of all the grid cell. Access via m_velocity[i + j*width + k*width*height] */ 
            float    m_amplitude[2];      /*!< The maximum and minimum velocity amplitude */
    };
}

#endif
