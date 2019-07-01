#include "Datasets/Annotation/Annotation.h"

namespace sereno
{
    Annotation::Annotation(uint32_t width, uint32_t height, float* position) : m_width(width), m_height(height)
    {
        if(position)
            for(int i = 0; i < 3; i++)
                m_position[i] = position[i];
        else
            for(int i = 0; i < 3; i++)
                m_position[i] = 0;
    }

    AnnotationStroke* Annotation::emplaceStroke()
    {
        AnnotationStroke* stroke = new AnnotationStroke();
        m_strokes.push_back(std::shared_ptr<AnnotationStroke>(stroke));
        return stroke;
    }

    void Annotation::addStroke(std::shared_ptr<AnnotationStroke> stroke)
    {
        m_strokes.push_back(stroke);
    }

    AnnotationText* Annotation::emplaceText()
    {
        AnnotationText* text = new AnnotationText();
        m_texts.push_back(std::shared_ptr<AnnotationText>(text));
        return text;
    }

    void Annotation::addText(std::shared_ptr<AnnotationText> text)
    {
        m_texts.push_back(text);
    }
}
