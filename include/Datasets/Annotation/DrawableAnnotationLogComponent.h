#ifndef  DRAWABLEANNOTATIONLOGCOMPONENT_INC
#define  DRAWABLEANNOTATIONLOGCOMPONENT_INC

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>

#include "Datasets/Annotation/AnnotationLogContainer.h"
#include "Datasets/Annotation/AnnotationLogComponent.h"

namespace sereno
{
    /** \brief  Class containing data information for drawing AnnotationLogComponent
     * @tparam T the type of the component to use
     * This set of classes relies on AnnotationLogContainer which contains the internal data read and parsed from disk*/
    template<typename T>
    class DrawableAnnotationLogComponent
    {
        public:
            typedef T type;

            /** \brief  Constructor
             * \param container the container which has parsed the data component
             * \param component the actual component to render */
            DrawableAnnotationLogComponent(std::shared_ptr<AnnotationLogContainer> container, std::shared_ptr<T> component) : m_component(component), m_container(container) {}

            virtual ~DrawableAnnotationLogComponent() {};

            /** \brief  Get the annotation component possessing the data
             * \return  the annotation component data model */
            std::shared_ptr<T> getData() {return m_component;}

            /** \brief  Should we consider the time component?
             * \param s true if yes, false otherwise */
            void setEnableTime(bool s) {m_enableTime = s;}

            /** \brief  Should we consider the time component?
             * \return true if yes, false otherwise */
            bool getEnableTime() const {return m_enableTime;}

            /** \brief  Is the time component used? This is different than "getEnableTime" as it considers, as well, the time column of the attached component
             * \return   true if the time should be used, false otherwise */
            bool isTimeUsed() const {return m_component != nullptr && m_component->getAnnotationLog()->getTimeInd() >= 0 && m_enableTime;}
        protected:
            bool m_enableTime = true;
            std::shared_ptr<T>                      m_component;
            std::shared_ptr<AnnotationLogContainer> m_container;
    };
}

#endif
