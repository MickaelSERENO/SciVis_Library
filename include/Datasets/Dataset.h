#ifndef  DATASET_INC
#define  DATASET_INC

#include "Datasets/SubDataset.h"
#include "Datasets/PointFieldDesc.h"
#include <vector>
#include <cstdint>

namespace sereno
{
    class Dataset;

    /* \brief  Callback function to call
     *
     * \param dataset  the Dataset which were loading data
     * \param status   the status of the loading
     * \param clbkData External data needed */
    typedef void(*LoadCallback)(Dataset* dataset, uint32_t status, void* clbkData);

    /** \brief  Dataset class. */
    class Dataset
    {
        public:
            /** \brief  Default constructor */
            Dataset(){}

            /* \brief  Copy constructor
             * \param copy argument to copy*/
            Dataset(const Dataset& copy);

            /* \brief  Movement constructor
             * \param mvt the object to move */
            Dataset(Dataset&& mvt);

            /* \brief  Copy operator
             * \param copy the argument to copy
             * \return   *this */
            Dataset& operator=(const Dataset& copy);

            /** \brief  Destructor */
            virtual ~Dataset();

            /* \brief  Get the SubDatasets registered
             * \return  The array of SubDatasets registered */
            const std::vector<SubDataset*>& getSubDatasets() const
            {
                return m_subDatasets;
            }

            /* \brief  Get the SubDatasets registered
             * \return  The array of SubDatasets registered */
            std::vector<SubDataset*>& getSubDatasets()
            {
                return m_subDatasets;
            }

            /**
             * \brief  Get the subdataset indice i
             * \param  i the indice of the subdataset required
             * \return  the SubDataset Having the correct ID
             */
            SubDataset* getSubDataset(uint32_t i) 
            {
                for(SubDataset* sd : m_subDatasets)
                    if(sd->getID() == i)
                        return sd;
                return NULL;
            }

            /**
             * \brief  Get the number of subdatasets this dataset possesses
             * \return  the number of subdataset this dataset possesses 
             */
            uint32_t getNbSubDatasets() const {return m_subDatasets.size();}

            /**
             * \brief Duplicated a given SubDataset: create a copy of it
             * \param sd the SubDataset to copy
             *
             * \return the new created SubDataset */
            virtual void duplicateSubDataset(const SubDataset& sd)
            {
                if(sd.getParent() == this)
                {
                    SubDataset* copy = new SubDataset(sd);
                    addSubDataset(copy);
                }
            }

            /* \brief  Remove a given SubDataset from the list of data
             * \param sd The SubDataset to remove*/
            virtual void removeSubDataset(SubDataset* sd) 
            {
                if(m_subDatasets.size() == 0)
                    return;

                for(std::vector<SubDataset*>::iterator it = m_subDatasets.begin(); it != m_subDatasets.end(); it++)
                {
                    if(*it == sd)
                    {
                        m_subDatasets.erase(it);
                        delete sd;
                        break;
                    }
                }
            }

            /* \brief  Add a SubDataset in the related SubDataset list. The parent must coincide
             * \param sd The SubDataset related */
            void addSubDataset(SubDataset* sd)
            {
                //Check if the SubDataset is already registered or not
                for(SubDataset* sd2 : m_subDatasets)
                    if(sd2 == sd)
                        return;

                sd->setID(m_curSDID++);
                m_subDatasets.push_back(sd);
            }

            /* \brief Add a SubDataset in the related SubDataset list. The parent must coincide. The SubDataset's ID will not be modified. Hence, to not use this function with addSubDataset since the latest changes the SubDataset ID, which can create conflict
             * \param sd The SubDataset related */
            void addSubDatasetWithID(SubDataset* sd)
            {
                for(SubDataset* sd2 : m_subDatasets)
                    if(sd2 == sd)
                        return;

                m_subDatasets.push_back(sd);
            }

            /* \brief  Get the description of point field data
             * \return  A point field description */
            const std::vector<PointFieldDesc>& getPointFieldDescs() const
            {
                return m_pointFieldDescs;
            }

            /* \brief  Load the values of the Dataset in a separated thread.
             * \param clbk the callback function to call when the loading is finished
             * \param data extra data to send to the callback function*/
            virtual void loadValues(LoadCallback clbk, void* data) = 0;

            /* \brief  Are the values loaded?
             * \return   true if yes, false otherwise */
            bool areValuesLoaded() const {return m_valuesLoaded;}

            /**
             * \brief  Create a 1D histogram
             *
             * \param output The output image. size: width*sizeof(uint32_t).
             * \param width  The output image width.  nbBinsX = (xAxis->max - xAxis->min)/width
             * \param ptFieldXID the point field ID to fetch
             *
             * \return true on success, false on failure. If failed, output will not be touched */
            virtual bool create1DHistogram(uint32_t* output, uint32_t width, uint32_t ptFieldXID) const = 0;

            /**
             * \brief  Create a 2D histogram
             *
             * \param output The output image. size: width*height*sizeof(uint32_t). X values are stocked first (row-major)
             * \param width  The output image width.  nbBinsX = (xAxis->max - xAxis->min)/width
             * \param height The output image height. nbBinsY = (yAxis->max - yAxis->min)/height
             * \param ptFieldXID the point field X ID to fetch
             * \param ptFieldXID the point field Y ID to fetch
             *
             * \return true on success, false on failure. If failed, output will not be touched */
            virtual bool create2DHistogram(uint32_t* output, uint32_t width, uint32_t height, uint32_t ptFieldXID, uint32_t ptFieldYID) const = 0;
        protected:
            /**
             * \brief  Set the subdataset amplitude using friendship
             * \param dataset the subdataset to modify
             * \param amplitude the new amplitude array
             */
            void setSubDatasetAmplitude(SubDataset* dataset, float* amplitude)
            {
                dataset->m_amplitude[0] = amplitude[0];
                dataset->m_amplitude[1] = amplitude[1];
            }

            /** \brief  Set the subdataset validity using friendship
             * \param dataset the subdataset to modify
             * \param isValid the new validity*/
            void setSubDatasetValidity(SubDataset* dataset, bool isValid)
            {
                dataset->m_isValid = isValid;
            }

            std::vector<SubDataset*>    m_subDatasets;     /*!< Array of sub datasets*/
            std::vector<PointFieldDesc> m_pointFieldDescs; /*!< Array of point field data*/
            uint32_t m_curSDID = 0; /*!< The current SubDataset ID*/
            bool     m_valuesLoaded = false; /*!< Are the values parsed?*/
    };
}

#endif
