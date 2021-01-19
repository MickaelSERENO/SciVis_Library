#ifndef  ANNOTATIONLOGCOMPONENT_INC
#define  ANNOTATIONLOGCOMPONENT_INC

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Datasets/Annotation/AnnotationLog.h"

namespace sereno
{
    /** \brief  Describe an annotation component from log */
    class AnnotationLogComponent
    {
        public:
            /** \brief  Constructor
             * \param ann the log information */
            AnnotationLogComponent(const AnnotationLog* ann) : m_ann(ann) {}

            /** \brief  Destructor */
            virtual ~AnnotationLogComponent(){}

            /** \brief  Should we consider the time component?
             * \param s true if yes, false otherwise */
            void setEnableTime(bool s) {m_enableTime = s;}

            /** \brief  Should we consider the time component?
             * \return true if yes, false otherwise */
            bool getEnableTime() const {return m_enableTime;}

            /** \brief  Get the annotation log being read
             * \return the annotation log */
            const AnnotationLog* getAnnotationLog() const {return m_ann;}
        protected:
            const AnnotationLog* m_ann;
            bool                 m_enableTime = true;
    };
}

#endif
