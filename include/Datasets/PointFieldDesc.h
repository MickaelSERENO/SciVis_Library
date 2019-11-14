#ifndef  POINTFIELDDESC_INC
#define  POINTFIELDDESC_INC

#include <string>
#include <cstdint>
#include <memory>
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
        uint32_t    id;                              /*!< The point field ID as defined by the Dataset*/
        float    minVal;                             /*!< The point field minimum value*/
        float    maxVal;                             /*!< The point field maximum value*/
        std::unique_ptr<void, _FreeDeleter> values;  /*!< The raw value pointers. The value was allocated using malloc. For vector, values == magnitude values*/
        std::unique_ptr<void, _FreeDeleter> gradVal; /*!< The gradient values raw pointer. The value was allocated using malloc. We used the magnitude for vectors as input for the gradient */
        float    maxGrad = 0;                        /*!< The maximum gradient value*/
    };
}

#endif
