#ifndef  POINTFIELDDESC_INC
#define  POINTFIELDDESC_INC

#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <limits>
#include "VTKParser.h"

namespace sereno
{
    struct _FreeDeleter{
        template <typename T>
        void operator()(T *p) const 
        {
            std::free(const_cast<std::remove_const_t<T>*>(p));
        }
    };

    /** \brief  Descriptor of point field. It stores point field meta data */
    struct PointFieldDesc : public FieldValueMetaData
    {
        uint32_t id;                                         /*!< The point field ID as defined by the Dataset*/
        float    minVal = std::numeric_limits<float>::max(); /*!< The point field minimum value*/
        float    maxVal = std::numeric_limits<float>::min(); /*!< The point field maximum value*/
        std::vector<std::shared_ptr<void>> values;           /*!< The raw value pointers of the data read from disk (usually) per timesteps*/
    };
}

#endif
