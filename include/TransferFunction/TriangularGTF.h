#ifndef  TRIANGULARGTF_INC
#define  TRIANGULARGTF_INC

#include "TransferFunction/TransferFunction.h"
#include <algorithm>

namespace sereno
{
    class TriangularGTF : public TF
    {
        public:
            /** \brief  Constructor. Set scale at 0.5f, center at 0.5 and alphaMax at 0.5 */
            TriangularGTF(uint32_t dim, ColorMode mode) : TF(dim, mode), m_alphaMax(0.5f)
            {
                m_scale  = (float*)malloc(sizeof(float)*(dim-1));
                m_center = (float*)malloc(sizeof(float)*(dim-1));
                for(uint32_t i = 0; i < dim-1; i++)
                {
                    m_scale[i]  = 0.5f;
                    m_center[i] = 0.5f;
                }
            }

            TriangularGTF(const TriangularGTF& copy) : TF(copy)
            {
                *this = copy;
            }

            TriangularGTF& operator=(const TriangularGTF& copy)
            {
                TF::operator=(copy);
                if(&copy != this)
                {
                    if(m_scale)
                        free(m_scale);
                    if(m_center)
                        free(m_center);
                    m_scale  = (float*)malloc(sizeof(float)*m_dim);    
                    m_center = (float*)malloc(sizeof(float)*m_dim);    

                    for(uint32_t i = 0; i < m_dim-1; i++)
                    {
                        m_scale[i]  = copy.m_scale[i];
                        m_center[i] = copy.m_center[i];
                    }
                }
                return *this;
            }

            ~TriangularGTF()
            {
                if(m_scale)
                    free(m_scale);
                if(m_center)
                    free(m_center);
            }

            /**
             * \brief  Compute the alpha component of the transfer function. The last value "ind" if for the gradient magnitude
             * The algorithm comes from "Gaussian Transfer Function for Multiple-Field Volume Visualization" by Kniss et al. in 2003 (IEEE Vis)
             *
             * \param ind the current Dim indice (i, j, k, ...). The last one is the gradient
             *
             * \return   the alpha computed
             */
            uint8_t computeAlpha(float* ind) const
            {
                if(ind[m_dim-1] < 1e-4)
                    return 0;

                float r0 = 1.f/ind[m_dim-1];
                float r1Mag = 0;

                for(uint32_t i = 0; i < m_dim-1; i++)
                {
                    if(m_scale[i] != 0)
                    {
                        float r = r0*(ind[i] - m_center[i])/m_scale[i];
                        r1Mag += r*r;
                    }
                }

                return std::min<float>(m_alphaMax*exp(-r1Mag)*255, 255.0f);
            }

            void computeColor(float* ind, uint8_t* colOut) const
            {
                float mag = 0;
                for(uint32_t i = 0; i < m_dim-1; i++)
                    mag += ind[i]*ind[i];
                mag = sqrt(mag)/(m_dim-1);

                //clip values
                if(mag < m_minClipping)
                    mag = 0.0f;
                else if(mag > m_maxClipping)
                    mag = 1.0f;
                else
                    mag = (mag - m_minClipping)/(m_maxClipping - m_minClipping);

                Color c = SciVis_computeColor(m_mode, mag);
                for(int i = 0; i < 3; i++)
                    colOut[i] = std::min(255.0f, std::max(0.0f, 255.0f*c[i]));
            }

            /**
             * \brief  Get the scale applied
             * \return  the scale 
             */
            const float* getScale()  const {return m_scale;}

            /**
             * \brief  Get the center applied
             * \return  the center 
             */
            const float* getCenter() const {return m_center;}

            /**
             * \brief  Get the alpha max applied
             * \return  the alpha max 
             */
            const float getAlphaMax() const {return m_alphaMax;}

            /**
             * \brief  Set the scaling along each axis of the TriangularGTF
             * \param scale the scaling along each axis of the TriangularGTF
             */
            void setScale(float* scale) 
            {
                for(uint8_t i = 0; i < m_dim-1; i++) 
                    m_scale[i] = scale[i];
            }
            /**
             * \brief  Set the center of the TriangularGTF
             * \param center the center of the TriangularGTF
             */
            void setCenter(float* center) 
            {
                for(uint8_t i = 0; i < m_dim-1; i++) 
                    m_center[i] = center[i];
            }
            /**
             * \brief  Set the alpha max of the TriangularGTF
             * \param alphaMax the alpha max
             */
            void setAlphaMax(float alphaMax) {m_alphaMax = alphaMax;}

            virtual bool hasGradient() const {return true;}

            virtual TF* clone()
            {
                return new TriangularGTF(*this);
            }
        private:
            float* m_scale    = NULL; /*!< The scaling factor*/
            float* m_center   = NULL; /*!< The center*/
            float  m_alphaMax;        /*!< The alpha max*/
    };
}

#endif
