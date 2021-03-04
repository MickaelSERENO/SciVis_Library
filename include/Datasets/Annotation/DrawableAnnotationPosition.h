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

            /** \brief  Set the default color that should represent this annotation position
             * \param color the new color to consider (R, G, B, A). Each component should range from 0.0 to 1.0 */
            void setColor(const glm::vec4& color) {m_color = color;}

            /** \brief  Get the default color that should represent this annotation position
             * \return the color to consider (R, G, B, A). Each component should range from 0.0 to 1.0 */
            const glm::vec4& getColor() const {return m_color;}

            /** \brief  Get the indices that can be used to color this drawable annotation based on the AnnotationLogContainer data
             * \return  The indices to look up on the AnnotationLogContainer object. Empty list == no data to look up. Hence, the position should only be represented using its default color */
            const std::vector<uint32_t>& getMappedDataIndices() const {return m_mappedIdx;}

            /** \brief Set the mapped data indices to read from the linked AnnotationLogContainer.
             * \param idx the new indices to use  */
            void setMappedDataIndices(const std::vector<uint32_t>& idx) {m_mappedIdx = idx;}
        private:
            glm::vec4             m_color     = glm::vec4(1.0, 1.0, 1.0, 1.0);
            std::vector<uint32_t> m_mappedIdx;
    };
}

#endif
