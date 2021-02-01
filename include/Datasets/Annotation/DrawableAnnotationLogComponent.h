#ifndef  DRAWABLEANNOTATIONLOGCOMPONENT_INC
#define  DRAWABLEANNOTATIONLOGCOMPONENT_INC

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Datasets/Annotation/AnnotationLogComponent.h"

namespace sereno
{
    class DrawableAnnotationLogComponent
    {
        public:
            virtual ~DrawableAnnotationLogComponent() = 0;

            /** \brief  Set the color that should represent this annotation position
             * \param color the new color to consider (R, G, B, A). Each component should range from 0.0 to 1.0 */
            void setColor(const glm::vec4& color) {m_color = color;}

            /** \brief  Get the color that should represent this annotation position
             * \return the color to consider (R, G, B, A). Each component should range from 0.0 to 1.0 */
            const glm::vec4& getColor() const {return m_color;}
        
            /** \brief  Should we consider the time component?
             * \param s true if yes, false otherwise */
            void setEnableTime(bool s) {m_enableTime = s;}

            /** \brief  Should we consider the time component?
             * \return true if yes, false otherwise */
            bool getEnableTime() const {return m_enableTime;}
        private:
            glm::vec4 m_color      = glm::vec4(1.0, 1.0, 1.0, 1.0);
            bool      m_enableTime = true;
    };
}

#endif
