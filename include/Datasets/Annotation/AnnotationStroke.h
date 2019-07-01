#ifndef  ANNOTATIONSTROKE_INC
#define  ANNOTATIONSTROKE_INC

#include <glm/glm.hpp>
#include <vector>

namespace sereno
{
    /** \brief  Annotation's stroke data */
    class AnnotationStroke
    {
        public:
            /* \brief  Constructor
             *
             * \param rgbaColor the RGBA stroke color (default: opaque black)
             * \param strokeWidth The stroke width (default: 5px) */
            AnnotationStroke(uint32_t rgbaColor = 0x000000FF, uint32_t strokeWidth = 5);

            /* \brief  Set the annotation stroke's color
             * \param rgbaColor the rgba stroke's color */
            void setColor(uint32_t rgbaColor) {m_rgbaColor = rgbaColor;}

            /* \brief  Get the stroke's RGBA color
             * \return    the stroke's RGBA color */
            uint32_t getColor() const {return m_rgbaColor;}

            /* \brief  Set the stroke's width 
             * \param pxWidth the stroke's width in pixels */
            void setStrokeWidth(uint32_t pxWidth) {m_strokeWidth = pxWidth;}

            /* \brief  Get the points registered to this stroke
             * \return   2D stroke's points */
            const std::vector<glm::ivec2>& getPoints() const {return m_points;}

            /* \brief  Add a new point on the registered points
             * \param point the new 2D point to add on this stroke.*/
            void addPoint(const glm::ivec2& point) {m_points.push_back(point);}
        private:
            uint32_t m_rgbaColor;   /*!< The stroke RGBA color*/
            uint32_t m_strokeWidth; /*!< The stroke pixel width*/
            std::vector<glm::ivec2> m_points; /*!< The stroke points. Each successive points create the stroke*/
    };
};

#endif
