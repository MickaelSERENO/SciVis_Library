#ifndef  SUBDATASET_INC
#define  SUBDATASET_INC

#include <limits>
#include <stdint.h>
#include <memory>
#include <list>
#include <glm/glm.hpp>
#include <string>
#include "Quaternion.h"
#include "TransferFunction/TransferFunction.h"
#include "ColorMode.h"
#include "Datasets/Annotation/Annotation.h"

#ifdef SNAPSHOT
    #include "Datasets/Snapshot.h"
#endif

namespace sereno
{
    class Dataset;

    /** \brief  Represent a dataset. Aims to be derived */
    class SubDataset
    {
        public:
            /** \brief  Constructor 
             * \param parent the parent Dataset
             * \param name the SUbDataset name*/
            SubDataset(Dataset* parent, const std::string& name);

            virtual ~SubDataset();

            /** \brief Set the clamping of this dataset at rendering time
             * \param min the minimum clamping
             * \param max the maximum clamping*/
            void setClamping(float min, float max);

            /* \brief Set the global rotation of this fluid dataset
             * \param quat the global rotation quaternion to apply */
            void setGlobalRotate(const Quaternionf& quat) {m_rotation = quat;}

            /* \brief Get the global rotation quaternion of this dataset
             * \return a reference to the global rotation quaternion of this dataset */
            const Quaternionf& getGlobalRotate() const {return m_rotation;}

            /* \brief Get the minimum clamping value in ratio (0.0, 1.0)
             * \return the minimum clamping value */
            float     getMinClamping() const {return m_minClamp;}

            /* \brief Get the maximum clamping value in ratio (0.0, 1.0)
             * \return the maximum clamping value */
            float     getMaxClamping() const {return m_maxClamp;}

            /* \brief Get the minimum amplitude of this dataset
             * \return the minimum amplitude */
            float getMinAmplitude() const {return m_amplitude[0];}

            /* \brief Get the maximum amplitude of this dataset
             * \return the maximum amplitude */
            float getMaxAmplitude() const {return m_amplitude[1];}

            /* \brief Is this dataset valid ? */
            bool isValid() const {return m_isValid;}

            /** \brief  Get the parent dataset
             * \return  the parent dataset */
            Dataset* getParent() {return m_parent;}

            /** \brief  Get the parent dataset
             * \return  the parent dataset */
            const Dataset* getParent() const {return m_parent;}

            /** \brief Set the sub dataset amplitude
             * \param amp the new amplitude*/
            void setAmplitude(float* amp) {m_amplitude[0] = amp[0]; m_amplitude[1] = amp[1];}

            /* \brief  Get the 3D position of this sub dataset
             * \return  const reference of the 3D position*/
            const glm::vec3& getPosition() const {return m_position;}

            /* \brief  Set the 3D position of this sub dataset
             * \param pos the new 3D position */
            void setPosition(const glm::vec3& pos) {m_position = pos;}

            /* \brief  Get the 3D scaling of this sub dataset
             * \return  The 3D (x, y, z) scaling of this sub dataset*/
            const glm::vec3& getScale() const {return m_scale;}

            /* \brief  Set the 3D scaling of this sub dataset
             * \param scale the new 3D scaling to apply */
            void setScale(const glm::vec3& scale) {m_scale = scale;}

            /* \brief  Get the SubDataset name
             * \return   The SubDataset name */
            const std::string& getName() {return m_name;}

            /* \brief  Emplace a new annotation
             * \param pxWidth the width in pixels of the annotation
             * \param pxHeight the height in pixels of the annotation
             * \param position the annotation's 3D position. Can be NULL (hence position = 0, 0, 0)
             * \return    the new created annotation. Do not delete/free it */
            Annotation* emplaceAnnotation(uint32_t pxWidth, uint32_t pxHeight, float* position);

            /* \brief  Add a new annotation. 
             * \param annot the new annotation to add. */
            void addAnnotation(std::shared_ptr<Annotation> annot);

            /* \brief  Get the registered annotations
             * \return  the registered annotations bound to this SubDataset */
            const std::list<std::shared_ptr<Annotation>>& getAnnotations() const {return m_annotations;}

#ifdef SNAPSHOT
            /**
             * \brief  Set the snapshot from this scientific visualization
             * \param snapshot the snapshot object
             */
            void setSnapshot(std::shared_ptr<Snapshot> snapshot)
            {
                m_snapshot = snapshot;
            }

            /* \brief Get the snapshot pixels ARGB8888. 
             * \return A pointer to the snapshot structure. */
            Snapshot* getSnapshot() const {return m_snapshot.get();}
#endif

            /* \brief  Get the transfer function bound to this subdataset
             * \return   The transfer function*/
            TF*       getTransferFunction() {return m_tf;}

            /* \brief  Get the transfer function bound to this subdataset
             * \return   The transfer function*/
            const TF* getTransferFunction() const {return m_tf;}

            /* \brief  Set the transfer function to use
             * \param tf the transfer function to use */
            void setTransferFunction(TF* tf) {m_tf = tf;}
        protected:
            bool        m_isValid        = false;              /*!< Is this dataset in a valid state ?*/
            float       m_minClamp       = 0.0f;               /*!< The minimum color clamping*/
            float       m_maxClamp       = 1.0f;               /*!< The maximum color clamping (ratio : 0.0f 1.0)*/
            float       m_amplitude[2];                        /*!< The dataset amplitude*/
            Quaternionf m_rotation;                            /*!< The quaternion rotation*/
            glm::vec3   m_position = glm::vec3(0.0, 0.0, 0.0); /*!< The small multiple position*/
            glm::vec3   m_scale    = glm::vec3(1.0, 1.0, 1.0); /*!< The 3D scaling*/
            Dataset*    m_parent   = NULL;                     /*!< The parent dataset*/
            std::string m_name;                                /*!< The SubDataset name*/
            TF*         m_tf       = NULL;                     /*!< The transfer function in application*/
            std::list<std::shared_ptr<Annotation>> m_annotations;              /*!< The SubDataset Annotation*/

#ifdef SNAPSHOT
            std::shared_ptr<Snapshot> m_snapshot;              /*!< The snapshot structure*/
#endif

        friend class Dataset;
    };
}

#endif
