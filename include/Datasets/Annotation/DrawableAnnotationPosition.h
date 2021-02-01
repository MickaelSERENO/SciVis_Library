#ifndef  DRAWABLEANNOTATIONPOSITION_INC
#define  DRAWABLEANNOTATIONPOSITION_INC

#include "Datasets/Annotation/DrawableAnnotationLogComponent.h"
#include <memory>

namespace sereno
{
    class DrawableAnnotationPosition : public DrawableAnnotationLogComponent
    {
        public:
            DrawableAnnotationPosition(std::weak_ptr<AnnotationPosition> pos) : m_pos(pos) {}
            ~DrawableAnnotationPosition(){}

            std::weak_ptr<AnnotationPosition> getData() {return m_pos;}
        private:
            std::weak_ptr<AnnotationPosition> m_pos;
    };
}

#endif
