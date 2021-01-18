#ifndef  ANNOTATIONLOGCONTAINER_INC
#define  ANNOTATIONLOGCONTAINER_INC

#include "Datasets/Annotation/AnnotationLog.h"
#include "Datasets/Annotation/AnnotationPosition.h"
#include <map>
#include <memory>
#include <vector>
#include <string>

#define ANNOTATION_LOG_CONTAINER_ERROR_HEADER_ALREADY_PRESENT -1
#define ANNOTATION_LOG_CONTAINER_INVALID_PARENT               -2


namespace sereno
{
    /** \brief  A container of annotation log information */
    class AnnotationLogContainer : public AnnotationLog
    {
        public:
            /** \brief  Initialize the Log reading
             * \param header should we expect a header when reading data?  */
            AnnotationLogContainer(bool header=true) : AnnotationLog(header) {}
            
            /** \brief  Destructor */
            virtual ~AnnotationLogContainer();

            /** \brief  Build an annotation positional view based on this Annotation Log. The returned values can then be parameterized and be sent to "parseAnnotationPosition" function for storing and reading its values.
             *
             * \return  An AnnotationPosition that can be configured and then read in "parseAnnotationPosition" function. */
            std::shared_ptr<AnnotationPosition> buildAnnotationPositionView() const;

            /** \brief parse an annotation position configured with "this" set as the annotation log
             * \param annot the annotation to parse
             * \return  0 on success, an error message otherwise. See "ANNOTATION_LOG_CONTAINER_ERROR" for more details*/
            int32_t parseAnnotationPosition(std::shared_ptr<AnnotationPosition> annot);

            /** \brief  Get a map of all the registered annotation position and the associated read position.
             * \return   all the parsed annotation positions and the corresponding positions*/
            const std::map<std::shared_ptr<AnnotationPosition>, std::vector<glm::vec3>*>& getAnnotationPositions() const {return m_positions;}

            /** \brief  Get the positions to use from an AnnotationPosition view.
             * \param annot the view to look after
             * \return   the associated 3D positions. NULL if AnnotationPosition was not found  */
            const std::vector<glm::vec3>* getPositionsFromView(std::shared_ptr<AnnotationPosition> annot) const;

            /** \brief  Get the positions to use from an AnnotationPosition view.
             * \param annot the view to look after
             * \return   the associated 3D positions that can be modified. NULL if AnnotationPosition was not found  */
            std::vector<glm::vec3>* getPositionsFromView(std::shared_ptr<AnnotationPosition> annot) {return const_cast<std::vector<glm::vec3>*>(std::as_const(*this).getPositionsFromView(annot));}

            /** \brief  Get the headers that are already assigned
             * \return  The already assigned headers. The list is sorted. */
            const std::vector<uint32_t>& getAssignedHeaders() const {return m_assignedHeaders;}

            /** \brief  By elimination, provides a list of headers yet-to assign
             * \return    the headers yet-to assign */
            std::vector<uint32_t> getRemainingHeaders() const;

            /** \brief  Get the stored time once parsed
             * \return   the time */
            const std::vector<float>& getParsedTime() const {return m_time;}

            /** \brief  Get the stored time once parsed
             * \return   the time */
            std::vector<float>& getParsedTime() {return m_time;}

            virtual void onParse();
            virtual void onSetTimeColumn();
        private:
            void readTimeValues();
            std::map<std::shared_ptr<AnnotationPosition>, std::vector<glm::vec3>*> m_positions;
            std::vector<uint32_t> m_assignedHeaders;
            int32_t               m_curTimeHeader = -1;
            std::vector<float>    m_time;
    };
}

#endif
