#ifndef  SUBDATASET_INC
#define  SUBDATASET_INC

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#include <limits>
#include <stdint.h>
#include <memory>
#include <list>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <string>
#include "Quaternion.h"
#include "TransferFunction/TransferFunction.h"
#include "ColorMode.h"
#include "Datasets/Annotation/AnnotationCanvas.h"
#include "Datasets/Annotation/AnnotationLogContainer.h"
#include "Datasets/Annotation/DrawableAnnotationPosition.h"

#ifdef SNAPSHOT
    #include "Datasets/Snapshot.h"
#endif

namespace sereno
{
    class Dataset;
    class SubDatasetGroup;

    /** \brief  Represent a dataset. Aims to be derived */
    class SubDataset
    {
        public:
            /** \brief  Constructor 
             * \param parent the parent Dataset
             * \param name the SubDataset name*/
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

            /** \brief  Set the SUbDataset name
             * \param name the new name to use */
            void setName(const std::string& name) {m_name = name;}

            /* \brief  Emplace a new canvas annotation
             * \param pxWidth the width in pixels of the annotation
             * \param pxHeight the height in pixels of the annotation
             * \param position the annotation's 3D position. Can be NULL (hence position = 0, 0, 0)
             * \return    the new created annotation. Do not delete/free it */
            AnnotationCanvas* emplaceAnnotationCanvas(uint32_t pxWidth, uint32_t pxHeight, float* position);

            /* \brief  Add a new annotation based on logged information 
             * \param annot the new annotation to add. */
            void addDrawableAnnotationPosition(std::shared_ptr<DrawableAnnotationPosition> annot) {m_annotationPositions.push_back(annot);}

            /* \brief  Add a new canvas annotation. 
             * \param annot the new annotation to add. */
            void addAnnotationCanvas(std::shared_ptr<AnnotationCanvas> annot);

            /* \brief Remove a canvas annotation in the list
             * \param annot the annotation to remove 
             * \return true on success, false on error*/
            bool removeAnnotationCanvas(std::shared_ptr<AnnotationCanvas> annot);

            /* \brief Remove an annotation in the list
             * \param it the iterator pointing to the annotation to remove
             * \return the next iterator in the list of the annotation (result of erase)*/
            std::list<std::shared_ptr<DrawableAnnotationPosition>>::const_iterator removeDrawableAnnotationPosition(std::list<std::shared_ptr<DrawableAnnotationPosition>>::const_iterator it) { return m_annotationPositions.erase(it); }

            /* \brief  Get the registered annotation positional informations
             * \return  the registered annotations bound to this SubDataset */
            const std::list<std::shared_ptr<DrawableAnnotationPosition>>& getDrawableAnnotationPositions() const {return m_annotationPositions;}

            /* \brief  Get the registered annotation positional information
             * \return  the registered annotations bound to this SubDataset */
            std::list<std::shared_ptr<DrawableAnnotationPosition>>& getDrawableAnnotationPositions() {return m_annotationPositions;}

            /* \brief Remove an annotation in the list
             * \param it the iterator pointing to the annotation to remove
             * \return the next iterator in the list of the annotation (result of erase)*/
            std::list<std::shared_ptr<AnnotationCanvas>>::const_iterator removeAnnotationCanvas(std::list<std::shared_ptr<AnnotationCanvas>>::const_iterator it);

            /* \brief  Get the registered canvas annotations
             * \return  the registered annotations bound to this SubDataset */
            const std::list<std::shared_ptr<AnnotationCanvas>>& getAnnotationCanvas() const {return m_annotationCanvases;}

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
            std::shared_ptr<const Snapshot> getSnapshot() const {return m_snapshot;}
#endif

            /* \brief  Get the transfer function bound to this subdataset
             * \return   The transfer function*/
            std::shared_ptr<TF> getTransferFunction() {return m_tf;}

            /* \brief  Get the transfer function bound to this subdataset
             * \return   The transfer function*/
            std::shared_ptr<const TF> getTransferFunction() const {return m_tf;}

            /* \brief  Set the transfer function to use
             * \param tf the transfer function to use */
            void setTransferFunction(std::shared_ptr<TF> tf) {m_tf = tf;}

            /** \brief  Get the volumetric mask. We are using bit mask and not boolean objects. Size: (getParent()->getNbSpatialData()+7)/8, see getVolumetricMaskSize
             * \return   the volumetric mask.  */
            const uint8_t* getVolumetricMask() const {return m_volumetricMask;}

            /** \brief  Get the volumetric mask. We are using bit mask and not boolean objects. Size: (getParent()->getNbSpatialData()+7)/8, see getVolumetricMaskSize
             * \return   the volumetric mask.  */
            uint8_t* getVolumetricMask() {return m_volumetricMask;}

            /** \brief  Get the size of the volumetric mask binary array
             * \return  The size of getVolumetricMask() array */
            size_t getVolumetricMaskSize() const;

            /** \brief Set the volumetric mask cell at x
             * \param x the spatial data indice to modify. The number of spatial value is getParent()->getNbSpatialData()
             * \param b the boolean status to apply*/
            void setVolumetricMaskAt(uint32_t x, bool b)
            {
                if(b)
                    m_volumetricMask[x/8] = m_volumetricMask[x/8] | (1 << (x%8));
                else
                    m_volumetricMask[x/8] = m_volumetricMask[x/8] & (~(1 << (x%8)));
            }

            /** \brief Get the volumetric mask cell at x
             * \param x the spatial data indice to get. The number of spatial value is getParent()->getNbSpatialData()
             * \return the mask value. true for activated, false for disactivated*/
            bool getVolumetricMaskAt(uint32_t x) const
            {
                return m_volumetricMask[x/8] & (1 << (x%8));
            }

            /** \brief  Reset to false or true the volumetric mask 
             * \param t the reset value (false or true)
             * \param isEnabled should we enable this volumetric mask?*/
            void resetVolumetricMask(bool t, bool isEnabled=true);

            /** \brief  If the volumetric selection enabled?
             * \return   true if no, false otherwise */
            bool isVolumetricMaskEnabled() const {return m_enableVolumetricMask;}

            /** \brief  Enable/Disable the volumetric mask
             * \param e the enable value */
            void enableVolumetricMask(bool e) {m_enableVolumetricMask = e;}

            /** \brief  Get the min depth clipping value to use for this SubDataset
             * \return  the min depth clipping value that should be between 0.0f (totally clipped) to 1.0f (no clipping to apply) */
            float getMinDepthClipping() const {return m_minDepthClipping;}

            /** \brief  Get the min depth clipping value to use for this SubDataset
             * \return  the min depth clipping value that should be between 0.0f (totally clipped) to 1.0f (no clipping to apply) */
            float getMaxDepthClipping() const {return m_maxDepthClipping;}

            /** \brief  Set the depth clipping value to use for this SubDataset. Values shall be clamped between 0.0f and 1.0f 
             * \param minD the min depth clipping values.
             * \param maxD the max depth clipping values.*/
            void setDepthClipping(float minD, float maxD) {m_minDepthClipping = std::max(std::min(minD, 1.0f), 0.0f); m_maxDepthClipping = std::max(std::min(maxD, 1.0f), 0.0f);}

            /** \brief  Get the ID of this SubDataset
             * \return   the subdataset ID */
            uint32_t getID() const {return m_id;}
            
            /** \brief  Get the model--world matrix
             * \return  the glm::mat4 associated */
            glm::mat4 getModelWorldMatrix() const;

            /** \brief  Set the SubDatasetGroup linked to this SubDataset.
             * \param group The SubDatasetGroup to consider. nullptr == no SubDatasetGroup to consider */
            void setSubDatasetGroup(SubDatasetGroup* group);

            /** \brief Get the SubDatasetGroup linked to this SubDataset 
             * \return A valid pointer to the linked SubDatasetGroup. nullptr == no SubDatasetGroup linked */
            SubDatasetGroup* getSubDatasetGroup() {return m_sdGroup;}

            /** \brief Get the SubDatasetGroup linked to this SubDataset 
             * \return A valid pointer to the linked SubDatasetGroup. nullptr == no SubDatasetGroup linked */
            const SubDatasetGroup* getSubDatasetGroup() const {return m_sdGroup;}
        protected:
            bool        m_isValid        = false;              /*!< Is this dataset in a valid state ?*/
            Quaternionf m_rotation;                            /*!< The quaternion rotation*/
            glm::vec3   m_position = glm::vec3(0.0, 0.0, 0.0); /*!< The small multiple position*/
            glm::vec3   m_scale    = glm::vec3(1.0, 1.0, 1.0); /*!< The 3D scaling*/
            Dataset*    m_parent   = NULL;                     /*!< The parent dataset*/
            std::string m_name;                                /*!< The SubDataset name*/
            std::shared_ptr<TF> m_tf       = NULL;                     /*!< The transfer function in application*/
            uint32_t    m_id       = 1;                        /*!< The SubDataset ID*/
            std::list<std::shared_ptr<AnnotationCanvas>>           m_annotationCanvases;  /*!< The SubDataset's AnnotationCanvas*/
            std::list<std::shared_ptr<DrawableAnnotationPosition>> m_annotationPositions; /*!< The SubDataset's AnnotationLog*/

#ifdef SNAPSHOT
            std::shared_ptr<Snapshot> m_snapshot; /*!< The snapshot structure*/
#endif
            uint8_t* m_volumetricMask       = NULL;  /*!< The volumetric mask*/
            bool     m_enableVolumetricMask = false; /*!< Should we consider the SubDataset volumetric mask?*/

            float    m_minDepthClipping        = 0.0f;  /*!< The min depth clipping value to use for this SubDataset*/
            float    m_maxDepthClipping        = 1.0f;  /*!< The max depth clipping value to use for this SubDataset*/

            SubDatasetGroup* m_sdGroup = nullptr;
        private:
            /* \brief  Set the ID of this SubDataset. This method is mostly aimed at being called by the Dataset class.
             * \param id the new ID */
            void setID(uint32_t id) {m_id = id;}

        friend class Dataset;
        friend class SubDatasetGroup;
    };
}

#endif
