#ifndef  DATASETMETADATA_INC
#define  DATASETMETADATA_INC

#include <string>
#include <vector>

namespace sereno
{
    /** \brief  Per timestep meta data. */
    struct PerTimestepMetadata
    {
        std::string date; /*!< The date string representing this timestep */
    };

    /** \brief  Structure containing data read from the meta data file associated to a specific dataset */
    struct DatasetMetadata
    {
        public:
            /** \brief  Default constructor, no metadata is filled */
            DatasetMetadata(){}

            /** \brief  Constructor
             * \param filePath the metadata file to read */
            DatasetMetadata(const std::string& filePath);

            std::string coastlinePath; /*!< The coastline file name*/
            std::vector<PerTimestepMetadata> perTimestepMetadata; /*!< Metadata per timestep*/
    };
}

#endif
