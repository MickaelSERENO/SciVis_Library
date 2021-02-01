#ifndef  ANNOTATIONLOGCOMPONENT_INC
#define  ANNOTATIONLOGCOMPONENT_INC

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Datasets/Annotation/AnnotationLog.h"
#include <list>

namespace sereno
{
    class AnnotationLogComponent;

    /** \brief  The listener interface to call for AnnotationLogComponent modifications*/
    class AnnotationLogComponentListener
    {
        public:
            /** \brief  Method to call when the headers used for the data has changed 
             * \param component the component calling this method
             * \param oldHeaders the old component's headers*/
            virtual void onUpdateHeaders(AnnotationLogComponent* component, const std::vector<int32_t>& oldHeaders) = 0;
    };

    /** \brief  Describe an annotation component from log */
    class AnnotationLogComponent
    {
        public:
            /** \brief  Constructor
             * \param ann the log information. The Annotation object should not outlive this annotation object */
            AnnotationLogComponent(const AnnotationLog* ann) : m_ann(ann) {}

            /** \brief  Destructor */
            virtual ~AnnotationLogComponent(){}

            /** \brief  Add a listener to consider each time this object is modified
             * \param l the object to call on events
             * \return   true if this listener was not already added. Otherwise, the function returns false and the object is not added */
            bool addListener(AnnotationLogComponentListener* l);

            /** \brief  Remove an already registered listener which we considered each time this object is modified
             * \param l the object to stop calling on events
             * \return   true if this listener was already added. Otherwise, the function returns false and nothing is done*/
            bool removeListener(AnnotationLogComponentListener* l);

            /** \brief  Get the annotation log being read
             * \return the annotation log */
            const AnnotationLog* getAnnotationLog() const {return m_ann;}

            /** \brief  Get the headers this component is using
             * \return   The headers ID this component consumes*/
            virtual std::vector<int32_t> getHeaders() const {return {};}
        protected:
            void callOnUpdateHeaders(const std::vector<int32_t>& oldHeaders) 
            {
                for(auto it : m_listeners)
                    it->onUpdateHeaders(this, oldHeaders);
            }
            std::list<AnnotationLogComponentListener*> m_listeners;
            const AnnotationLog*                       m_ann;
    };
}

#endif
