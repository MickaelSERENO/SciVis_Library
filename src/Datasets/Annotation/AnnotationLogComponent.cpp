#include "Datasets/Annotation/AnnotationLogComponent.h"
#include <algorithm>

namespace sereno
{
    bool AnnotationLogComponent::addListener(AnnotationLogComponentListener* l)
    {
        auto it = std::find(m_listeners.begin(), m_listeners.end(), l);
        if(it == m_listeners.end())
        {
            m_listeners.push_back(l);
            return true;
        }
        return false;
    }

    bool AnnotationLogComponent::removeListener(AnnotationLogComponentListener* l)
    {
        auto it = std::find(m_listeners.begin(), m_listeners.end(), l);
        if(it != m_listeners.end())
        {
            m_listeners.erase(it);
            return true;
        }
        return false;
    }
}
