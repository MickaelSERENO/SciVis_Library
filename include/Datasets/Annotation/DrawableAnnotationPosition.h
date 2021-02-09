#ifndef  DRAWABLEANNOTATIONPOSITION_INC
#define  DRAWABLEANNOTATIONPOSITION_INC

#include "Datasets/Annotation/DrawableAnnotationLogComponent.h"
#include <memory>

namespace sereno
{
    class DrawableAnnotationPosition : public DrawableAnnotationLogComponent<AnnotationPosition>
    {
        public:
            DrawableAnnotationPosition(std::shared_ptr<AnnotationLogContainer> container, std::shared_ptr<AnnotationPosition> pos) : DrawableAnnotationLogComponent(container, pos) {}
            ~DrawableAnnotationPosition(){}

            /** \brief  Set the color that should represent this annotation position
             * \param color the new color to consider (R, G, B, A). Each component should range from 0.0 to 1.0 */
            void setColor(const glm::vec4& color) {m_color = color;}

            /** \brief  Get the color that should represent this annotation position
             * \return the color to consider (R, G, B, A). Each component should range from 0.0 to 1.0 */
            const glm::vec4& getColor() const {return m_color;}
        private:
            glm::vec4 m_color = glm::vec4(1.0, 1.0, 1.0, 1.0);
    };
}

#endif
