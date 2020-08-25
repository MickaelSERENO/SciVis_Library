#ifndef  MERGETF_INC
#define  MERGETF_INC

#include "TransferFunction/TransferFunction.h"
#include <memory>

namespace sereno
{
    /** \brief  Merging Transfer Function. Use two transfer function and merge them using a t slider */
    class MergeTF : public TF
    {
        public:
            /**
             * \brief  Constructor, initialize the object with two initial transfer function and one t. It is not necessary to have the same dimension for both transfer function. The dimension of the object is the highest possible dimension between the two transfer functions given in parameter.
             *
             * \param tf1 the first transfer function (at t==0.0f)
             * \param tf2 the second transfer function (at t==1.0f)
             * \param t the linear interpolation parameter. Must be between 0.0f and 1.0f*/
            MergeTF(std::shared_ptr<TF> tf1, std::shared_ptr<TF> tf2, float t=0.0f) : TF(std::max(tf1->getDimension(), tf2->getDimension()), tf1->getColorMode()),  m_tf1(tf1), m_tf2(tf2), m_t(t)
            {
                if(m_tf1->getDimension() <= m_tf2->getDimension() &&
                   m_tf1->hasGradient() && !m_tf2->hasGradient())
                    m_dim+=1;

                else if(m_tf2->getDimension() <= m_tf1->getDimension() &&
                        m_tf2->hasGradient() && !m_tf1->hasGradient())
                    m_dim+=1;
            }

            MergeTF(const MergeTF& copy) : TF(copy)
            {
                *this = copy;
            }

            ~MergeTF() 
            {}

            virtual uint8_t computeAlpha(float* ind) const
            {
                uint8_t tf1Val = 0;
                uint8_t tf2Val = 0;
                
                //We need to rearrange "ind" because of the gradient of the lowest dimension object
                
                //Check tf1
                if(m_tf1->getDimension() < m_dim && m_tf1->hasGradient())
                {
                    float temp = ind[m_tf1->getDimension() - 1];
                    ind[m_tf1->getDimension()-1] = ind[m_dim-1];
                    tf1Val = m_tf1->computeAlpha(ind);
                    ind[m_tf1->getDimension()-1] = temp;
                }
                else
                    tf1Val = m_tf1->computeAlpha(ind);

                //Check tf2
                if(m_tf2->getDimension() < m_dim && m_tf2->hasGradient())
                {
                    float temp = ind[m_tf2->getDimension() - 1];
                    ind[m_tf2->getDimension()-1] = ind[m_dim-1];
                    tf2Val = m_tf2->computeAlpha(ind);
                    ind[m_tf2->getDimension()-1] = temp;
                }
                else
                    tf2Val = m_tf2->computeAlpha(ind);

                return (uint8_t)((1.0f-m_t)*tf1Val + m_t*tf2Val);
            }

            virtual void computeColor(float* ind, uint8_t* colOut) const
            {
                uint8_t tf1Val[3];
                uint8_t tf2Val[3];
                
                //We need to rearrange "ind" because of the gradient of the lowest dimension object
                
                //Check tf1
                if(m_tf1->getDimension() < m_dim && m_tf1->hasGradient())
                {
                    float temp = ind[m_tf1->getDimension() - 1];
                    ind[m_tf1->getDimension()-1] = ind[m_dim-1];
                    m_tf1->computeColor(ind, tf1Val);
                    ind[m_tf1->getDimension()-1] = temp;
                }
                else
                    m_tf1->computeColor(ind, tf1Val);

                //Check tf2
                if(m_tf2->getDimension() < m_dim && m_tf2->hasGradient())
                {
                    float temp = ind[m_tf2->getDimension() - 1];
                    ind[m_tf2->getDimension()-1] = ind[m_dim-1];
                    m_tf2->computeColor(ind, tf2Val);
                    ind[m_tf2->getDimension()-1] = temp;
                }
                else
                    m_tf2->computeColor(ind, tf2Val);

                for(uint8_t i = 0; i < 3; i++)
                    colOut[i] = (uint8_t)((1.0f-m_t)*tf1Val[i] + m_t*tf2Val[i]);
            }

            virtual bool hasGradient() const {return m_tf1->hasGradient() || m_tf2->hasGradient();}

            /** \brief  Set the interpolation t parameter
             * \param t the interpolation parameter. Must be between 0.0f and 1.0f. At t==0.0f, computes only tf1. At t==1.0f, computes only tf2 */
            void setInterpolationParameter(float t)
            {
                m_t = t;
            }

            /** \brief  Get the interpolation t parameter
             * \return the interpolation parameter. It is between 0.0f and 1.0f */
            float getInterpolationParameter() const 
            {
                return m_t;
            }

        private:
            std::shared_ptr<TF> m_tf1 = NULL; /*!< The first transfer function to interpolate at m_t==0.0f*/
            std::shared_ptr<TF> m_tf2 = NULL; /*!< The second transfer function to interpolate at m_t==1.0f*/
            float               m_t   = 0.0f; /*!< The linear interpolation parameter*/
    };
}

#endif
