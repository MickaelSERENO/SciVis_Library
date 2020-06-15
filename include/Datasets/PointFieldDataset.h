#ifndef  POINTFIELDDATASET_INC
#define  POINTFIELDDATASET_INC

#include "Datasets/Dataset.h"
#include <thread>
#include <string>

namespace sereno
{
    /** \brief  Load PointFieldDataset object. This object represent point with a float component associate to each point. No gradient can be derived */
    class PointFieldDataset : public Dataset
    {
        public:
            /* \brief  Constructor, load basic information
             * \param filePath the file of the dataset to load on "loadValues"*/
            PointFieldDataset(const std::string& filePath);

            /** \brief  Destructor */
            ~PointFieldDataset();

            /*----------------------------------------------------------------------------*/
            /*------------------------Virtual Inherited Functions-------------------------*/
            /*----------------------------------------------------------------------------*/
            void loadValues(LoadCallback clbk, void* data);
            bool create1DHistogram(uint32_t* output, uint32_t width, uint32_t ptFieldXID) const;
            bool create2DHistogram(uint32_t* output, uint32_t width, uint32_t height, uint32_t ptFieldXID, uint32_t ptFieldYID) const;

            /* \brief  Get the point positions 3D positions.
             * \return  If the dataset is loaded (see isLoaded()), returns a float array sized 3*getNbPoints(). Each tuple of 3 component represent a point of position (x, y, z). Else, return NULL */
            float const* getPointPositions();

            /* \brief  Get the point data. It corresponds to the first point field ID loaded (i.e., getPointFieldDescs()[0])
             * \return  If the dataset is loaded (see isLoaded()), returns a float array sized getNbPoints(). Each value represent the only point data. Else, return NULL*/
            float const* getPointData();

            /* \brief Get the number of points loaded 
             * \return  The number of points loaded */
            uint32_t getNbPoints();
        private:
            float*      m_positions = NULL; /*!< The 3D point positions (x1, y1, z1; x2, y2, z2; ...)*/
            uint32_t    m_nbPoints  = 0;    /*!< The number of pints loaded*/
            std::string m_filePath;         /*!< The file path to load*/

            std::thread                       m_readThread;        /*!< The reading thread*/
            bool                              m_readThreadRunning = false; /*!< Is the reading thread running?*/
    };
};

#endif
