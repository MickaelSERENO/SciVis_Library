#ifndef  SCIVIS_UTILS_INC
#define  SCIVIS_UTILS_INC

#include <strings.h>
#include <cstring>
#include <vector>
#include <mutex>

//Colors for printf
#ifndef RED
#define RED   "\x1B[31m"
#endif
#ifndef GRN
#define GRN   "\x1B[32m"
#endif
#ifndef YEL
#define YEL   "\x1B[33m"
#endif
#ifndef BLU
#define BLU   "\x1B[34m"
#endif
#ifndef MAG
#define MAG   "\x1B[35m"
#endif
#ifndef CYN
#define CYN   "\x1B[36m"
#endif
#ifndef WHT
#define WHT   "\x1B[37m"
#endif
#ifndef BOLD
#define BOLD  "\x1B[1m"
#endif
#ifndef RESET
#define RESET "\x1B[0m"
#endif

#ifndef __FILENAME__
#define __FILENAME__ (strrchr("/" __FILE__, '/') + 1)
#endif

//Stringification
#ifndef STR
#define STR_(x)  #x
#define STR(x)   STR_(x)
#endif

//Cat
#ifndef CAT
#define CAT_(x, y) x##y
#define CAT(x, y)  CAT_(x, y)
#endif

//Log system
#ifndef ERROR
#define ERROR   (std::cerr << BOLD RED "Error : "   RESET GRN  << __FILENAME__ << ":" STR(__LINE__) RESET " ")
#endif

#ifndef INFO
#define INFO    (std::cout << BOLD WHT "Info : "    RESET GRN  << __FILENAME__ << ":" STR(__LINE__) RESET " ")
#endif

#ifndef WARNING
#define WARNING (std::cout << BOLD YEL "Warning : " RESET GRN  << __FILENAME__ << ":" STR(__LINE__) RESET " ")
#endif

namespace sereno
{
    extern std::mutex ompMutex;
}

namespace serenoSciVis
{
    /* \brief Convert a uint8 ptr (4 value) to a uint32_t
     * \param data the uint8_t ptr
     * \return the uint32_t */
    inline uint32_t uint8ToUint32(uint8_t* data)
    {
        return (data[0] << 24) + (data[1] << 16) +
               (data[2] << 8 ) + (data[3]);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
    /* \brief Convert a uint8 ptr (4 value) to a float
     * \param data the uint8_t ptr
     * \return the float */
    inline float uint8ToFloat(uint8_t* data)
    {
        uint32_t t = uint8ToUint32(data);
        return *(float*)&t;
    }
#pragma GCC diagnostic pop


#if __cplusplus > 201703L
        /**
         * \brief  Create meta nested for loop. The most outer part of the for loop is at indice==Dim-1 
         *
         * @tparam Dim the dimension of the for loop
         * @tparam Callable the functor class callable
         * \param start array of where to start along each dimension
         * \param end array of where to finish along each dimension
         * \param c the function to call
         *
         * \return   
         */
        template<size_t Dim, class Callable>
        constexpr void metaForLoop(const size_t* start, const size_t* end, Callable&& c)
        {
            static_assert(Dim > 0);

            for(size_t i = start[Dim]; i != end[Dim]; i++)
            {
                if constexpr(Dim == 1)
                    c(i);
                else
                {
                    auto bindAnArgument = [i, &c](auto... args)
                    {
                        c(i, args...);
                    };
                    meta_for_loop<Dim-1>(begin, end, bindAnArgument);
                }
            }
        }
#endif
}

#endif
