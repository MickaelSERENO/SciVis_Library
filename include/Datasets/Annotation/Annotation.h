#ifndef  ANNOTATION_INC
#define  ANNOTATION_INC

#include <list>
#include <memory>
#include "Datasets/Annotation/AnnotationStroke.h"
#include "Datasets/Annotation/AnnotationText.h"

namespace sereno
{
    /** \brief  Annotation class, contains information about annotations */
    class Annotation
    {
        public:
            /* \brief  Constructor
             * \param pxWidth the width in pixels of the annotation
             * \param pxHeight the height in pixels of the annotation
             * \param position the annotation's 3D position. If NULL, position = (0, 0, 0) */
            Annotation(uint32_t pxWidth, uint32_t pxHeight, float* position);

            /** \brief  Destructor. */
            virtual ~Annotation(){}

            /* \brief Get the 3D Annotation's position 
             * \return   A pointer to a float array (size=3) containing the 3D position information (x, y, z)*/
            const float* getPosition() const {return m_position;}

            /* \brief  Set the 3D annotation's position
             * \param position the new 3D annotation's position */
            void setPosition(float* position) {for(int i = 0; i < 3; i++) m_position[i] = position[i];}

            /* \brief  Emplace a new stroke to take account
             * \return  The stroke added. Do not delete it */
            AnnotationStroke* emplaceStroke();

            /* \brief  Add a new stroke
             * \param stroke the stroke to add */
            void addStroke(std::shared_ptr<AnnotationStroke> stroke);

            /* \brief  Get the strokes registered for this annotation
             * \return  The annotation's strokes */
            const std::list<std::shared_ptr<AnnotationStroke>> getStrokes() const {return m_strokes;}

            /* \brief  Emplace a new text to take account
             * \return  The text added. Do not delete it */
            AnnotationText* emplaceText();

            /* \brief  Add a new text
             * \param text the text to add */
            void addText(std::shared_ptr<AnnotationText> text);

            /* \brief  Get the texts registered for this annotation
             * \return  The annotation's texts */
            const std::list<std::shared_ptr<AnnotationText>> getTexts() const {return m_texts;}

            /* \brief  Get the width in pixels of the annotation
             * \return   the annotation's width */
            uint32_t getWidth() const {return m_width;}

            /* \brief  Get the height in pixels of the annotation
             * \return   the annotation's height */
            uint32_t getHeight() const {return m_height;}
        private:
            uint32_t m_width;
            uint32_t m_height;
            float m_position[3]; /*!< The annotation 3D position*/
            std::list<std::shared_ptr<AnnotationStroke>> m_strokes; /*!< List of annotation strokes*/
            std::list<std::shared_ptr<AnnotationText>>   m_texts;   /*!< List of annotation texts*/
    };
}

#endif
