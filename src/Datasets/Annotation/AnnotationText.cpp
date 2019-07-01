#include "Datasets/Annotation/AnnotationText.h"

namespace sereno
{
    AnnotationText::AnnotationText(uint32_t rgbaColor, uint32_t* position, const std::string& text) : m_rgbaColor(rgbaColor), m_text(text)
    {
        if(position == NULL)
            for(int i = 0; i < 2; i++)
                m_position[i] = 0;
        else
            for(int i = 0; i < 2; i++)
                m_position[i] = position[i];
    }
}
