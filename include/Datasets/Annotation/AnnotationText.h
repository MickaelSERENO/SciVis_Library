#ifndef  ANNOTATIONTEXT_INC
#define  ANNOTATIONTEXT_INC

#include <string>

namespace sereno
{
    class AnnotationText
    {
        public:
            /**
             * \brief  Constructor
             *
             * \param rgbaColor the text's RGBA color
             * \param position the text's 2D position in pixel coordinate. NULL -> position = (0, 0)
             * \param text the text's text
             */
            AnnotationText(uint32_t rgbaColor = 0x000000FF, uint32_t* position=NULL, const std::string& text="");

            /* \brief  Set the annotation stroke's color
             * \param rgbaColor the rgba stroke's color */
            void setColor(uint32_t rgbaColor) {m_rgbaColor = rgbaColor;}

            /* \brief  Get the stroke's RGBA color
             * \return    the stroke's RGBA color */
            uint32_t getColor() const {return m_rgbaColor;}

            /* \brief  Get the 2D annotation's position in pixel coordinate
             * \return   the 2D annotation's position */
            const uint32_t* getPosition() const {return m_position;}

            /* \brief  Set the 2D annotation's position
             * \param position the new 2D annotation's position in pixel coordinate*/
            void setPosition(uint32_t* position) {for(int i = 0; i < 2; i++) m_position[i] = position[i];}

            /* \brief  Set the annotation's text
             * \param text the new annotation's text */
            void setText(const std::string& text) {m_text = text;}
            
            /* \brief  Get the annotation's text
             * \return    the annotation's text */
            const std::string& getText() const {return m_text;}            
        private:
            uint32_t    m_rgbaColor;
            uint32_t    m_position[2];
            std::string m_text;
    };
}

#endif
