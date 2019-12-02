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
            SubDataset(Dataset* parent, const std::string& name, uint32_t id);

            /** \brief Copy constructor
             * \param copy the parameter to copy */
            SubDataset(const SubDataset& copy);

            /** \brief Copy assignment
             * \param copy the parameter to copy*/
            SubDataset& operator=(const SubDataset& copy);

            virtual ~SubDataset();

            /* \brief Set the global rotation of this fluid dataset
             * \param quat the global rotation quaternion to apply */
            void setGlobalRotate(const Quaternionf& quat) {m_rotation = quat;}

            /* \brief Get the global rotation quaternion of this dataset
             * \return a reference to the global rotation quaternion of this dataset */
            const Quaternionf& getGlobalRotate() const {return m_rotation;}

            /* \brief Is this dataset valid ? */
            bool isValid() const {return m_isValid;}

            /** \brief  Get the parent dataset
             * \return  the parent dataset */
            Dataset* getParent() {return m_parent;}

            /** \brief  Get the parent dataset
             * \return  the parent dataset */
            const Dataset* getParent() const {return m_parent;}

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
            const std::string& getName() const {return m_name;}

            /* \brief  Emplace a new annotation
             * \param pxWidth the width in pixels of the annotation
             * \param pxHeight the height in pixels of the annotation
             * \param position the annotation's 3D position. Can be NULL (hence position = 0, 0, 0)
             * \return    the new created annotation. Do not delete/free it */
            Annotation* emplaceAnnotation(uint32_t pxWidth, uint32_t pxHeight, float* position);

            /* \brief  Add a new annotation. 
             * \param annot the new annotation to add. */
            void addAnnotation(std::shared_ptr<Annotation> annot);

            /* \brief Remove an annotation in the list
             * \param annot the annotation to remove 
             * \return true on success, false on error*/
            bool removeAnnotation(std::shared_ptr<Annotation> annot);

            /* \brief Remove an annotation in the list
             * \param it the iterator pointing to the annotation to remove
             * \return the next iterator in the list of the annotation (result of erase)*/
            std::list<std::shared_ptr<Annotation>>::const_iterator removeAnnotation(std::list<std::shared_ptr<Annotation>>::const_iterator it);

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

            uint32_t getID() const {return m_id;}
        protected:
            bool        m_isValid        = false;              /*!< Is this dataset in a valid state ?*/
            Quaternionf m_rotation;                            /*!< The quaternion rotation*/
            glm::vec3   m_position = glm::vec3(0.0, 0.0, 0.0); /*!< The small multiple position*/
            glm::vec3   m_scale    = glm::vec3(1.0, 1.0, 1.0); /*!< The 3D scaling*/
            Dataset*    m_parent   = NULL;                     /*!< The parent dataset*/
            std::string m_name;                                /*!< The SubDataset name*/
            TF*         m_tf       = NULL;                     /*!< The transfer function in application*/
            uint32_t    m_id       = 1;                        /*!< The SubDataset ID*/
            std::list<std::shared_ptr<Annotation>> m_annotations; /*!< The SubDataset Annotation*/

#ifdef SNAPSHOT
            std::shared_ptr<Snapshot> m_snapshot; /*!< The snapshot structure*/
#endif
        private:
            /* \brief  Set the ID of this SubDataset. This method is mostly aimed at being called by the Dataset class.
             * \param id the new ID */
            void setID(uint32_t id) {m_id = id;}

        friend class Dataset;
    };
}

#endif
