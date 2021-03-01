#ifndef  TRANSFERTFUNCTION_INC
#define  TRANSFERTFUNCTION_INC

#include <cstdint>
#include <cstdlib>
#include "SciVisColor.h"
#include <algorithm>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace sereno
{
    /** \brief  Basic class for transfer function computation */
    class TF
    {
        public:
            /* \brief  Constructor of Basic class of transfer functions
             * \param dim the dimension of the transfer function
             * \param mode the color mode*/
            TF(uint32_t dim, ColorMode mode) : m_dim(dim), m_mode(mode)
            {
                m_enabled.resize(m_dim, true);
            }

            TF(const TF& copy)
            {
                *this = copy;
            }

            TF& operator=(const TF& copy)
            {
                if(this != &copy)
                {
                    m_dim             = copy.m_dim;
                    m_mode            = copy.m_mode;
                    m_currentTimestep = copy.m_currentTimestep;
                    m_minClipping     = copy.m_minClipping;
                    m_maxClipping     = copy.m_maxClipping;
                }

                return *this;
            }

            virtual ~TF(){}

            /* \brief Compute the alpha component of this transfer function 
             * \param ind the normalized indice of the transfer function. Must be at least of size = getDim
             * \return the alpha component computed*/
            virtual uint8_t computeAlpha(float* ind) const
            {
                return 0xff;
            }

            /* \brief  Compute the RGB color of this transfer function. The color is the length of the indice
             * \param ind the normalized indice of the transfer function. Must be at least of size = getDim
             * \param colOut[out] the RGB color output. Minimum size: 3*/
            virtual void computeColor(float* ind, uint8_t* colOut) const
            {
                float mag = 0;
                for(uint32_t i = 0; i < m_dim; i++)
                {
                    float _ind = std::clamp(ind[i], m_minClipping, m_maxClipping);
                    mag += _ind*_ind;
                }
                mag = sqrt(mag)/m_dim;
                Color c = SciVis_computeColor(m_mode, mag);
                for(int i = 0; i < 3; i++)
                    colOut[i] = std::min(255.0f, std::max(0.0f, 255.0f*c[i]));
            }

            /* \brief  Get the transfer function dimension
             * \return   The transfer function dimension*/
            uint32_t getDimension() const {return m_dim;}

            /* \brief  Get the color mode of this transfer function
             * \return    the transfer function color mode */
            ColorMode getColorMode() const {return m_mode;}

            /* \brief  Get the color mode of this transfer function
             * \param mode the new transfer function color mode */
            void setColorMode(ColorMode mode) {m_mode = mode;}

            /* \brief  Is this Transfer function taking into account the gradient of the field?
             * \return  true if this transfer function uses the gradient of the field as a dimension, false otherwise */
            virtual bool hasGradient() const {return false;}

            /** \brief  set the array of the enabled dimensions.
             * \param ids the array of the enabled dimensions. ids[i] == dimensions[i].enabled (false if not enabled, true otherwise) */
            virtual void setEnabledDimensions(const std::vector<bool>& ids) {m_enabled = ids;}

            /** \brief  get the array of the enabled dimensions.
             * \return the enabled dimensions. array[i] == dimensions[i].enabled.*/
            const std::vector<bool>& getEnabledDimensions() const {return m_enabled;}

            /** \brief Get the current timestep to apply to your data visualization
             * \return the current timestep. */
            float getCurrentTimestep() const {return m_currentTimestep;}

            /** \brief Set the current timestep to apply to your data visualization
             * \param t the current timestep to apply. Must be positive. */
            void setCurrentTimestep(float t) {m_currentTimestep = t;}

            /** \brief Set the clipping values of this transfer function
             * \param min the minimum clipping value in the dimension format (between 0.0f and 1.0f). Default: 0.0f
             * \param max the maximum clipping value in the dimension format (between 0.0f and 1.0f). Default: 1.0f.*/
            void setClipping(float min, float max) 
            {
                m_minClipping = std::min(std::max(min, 0.0f), 1.0f);
                m_maxClipping = std::min(std::max(max, 0.0f), 1.0f);
                if(m_minClipping > m_maxClipping)
                    std::swap(m_minClipping, m_maxClipping);
            }

            /** \brief Get the min clipping value to use to adapt the indexes correctly 
             * \return The min clipping value in use*/
            float getMinClipping() const {return m_minClipping;}

            /** \brief Get the max clipping value to use to adapt the indexes correctly 
             * \return The max clipping value in use*/
            float getMaxClipping() const {return m_maxClipping;}
        protected:
            std::vector<bool> m_enabled;     /*!< m_enabled[ids] == true if enabled, false otherwise. Size: m_dim. */
            uint32_t  m_dim;                 /*!< The transfer function dimension*/
            ColorMode m_mode;                /*!< The color mode*/
            float     m_currentTimestep = 0; /*!< The current timestep*/
            float     m_minClipping     = 0;
            float     m_maxClipping     = 1;
    };

    /* \brief  Compute the transfer function texels. The dimension of the transfer function must be inferior at 1024
     * Use this function if you prefer parallelism. Otherwise use computeTFTexelsRec
     * \param texels[out] the texels to compute
     * \param texSize the size of the texture
     * \param tf the transfer function in use. Its dimension must be greater or equal to 1 */
    template <typename T>
    void computeTFTexels(uint8_t* texels, const uint32_t* texSize, const T& tf)
    {
        const int32_t ind = tf.getDimension()-1;
        uint32_t shift = 1;
        for(uint32_t i = 0; i < ind; i++)
            shift*=texSize[i];
        float indArr[1024];

#ifdef _OPENMP
        #pragma omp parallel private(indArr)
        {
            #pragma omp for
#endif
            for(uint32_t i = 0; i < texSize[ind]; i++)
            {
                indArr[ind] = ((float)i)/texSize[ind];
                computeTFTexelsRec(texels, texSize, indArr, tf, ind-1, i*shift);
            }
#ifdef _OPENMP
        }
#endif
    }

    /* \brief  Compute the transfer function texture by recursion. Some values are needed to be initialize at default value for the recursion to work
     *
     * \param texels[out] the texels to compute
     * \param texSize the size of the texture
     * \param indArr array of the stored indice (x, y, z, ...) while we iterate. Its size must be at least Dim. No needed to initialize it at the first call
     * \param tf the transfer function in use
     * \param ind current indice in the dimension. Go through dim-1 to 0. Must be dim-1 at the first call
     * \param off the offset of the texels array. Must be 0 at the first call*/
    template <typename T>
    void computeTFTexelsRec(uint8_t* texels, const uint32_t* texSize, float* indArr, 
                            const T& tf, int32_t ind, uint32_t off)
    {
        //Do the recursion
        if(ind > 0)
        {
            uint32_t shift = 1;
            for(uint32_t i = 0; i < ind; i++)
                shift*=texSize[i];
            for(uint32_t i = 0; i < texSize[ind]; i++)
            {
                indArr[ind] = ((float)i)/texSize[ind];
                computeTFTexelsRec(texels, texSize, indArr, tf, ind-1, off+i*shift);
            }
        }

        //Compute (finally) the RGBA components of the last dimension
        else
        {
            for(uint32_t i = 0; i < texSize[0]; i++)
            {
                indArr[0] = ((float)i)/texSize[0];

                //Compute the color
                uint8_t col[3];
                tf.computeColor(indArr, col);
                for(uint8_t j = 0; j < 3; j++)
                    texels[4*(off+i)+j] = col[j];

                //Compute the alpha component
                texels[4*(off+i)+3] = tf.computeAlpha(indArr);
            }
        }
    }
}

#endif
