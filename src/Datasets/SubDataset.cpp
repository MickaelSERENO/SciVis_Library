#include "Datasets/SubDataset.h"
#include "Datasets/Dataset.h"
#include "Datasets/SubDatasetGroup.h"
#include <cstring>

namespace sereno
{
    SubDataset::SubDataset(Dataset* parent, const std::string& name, uint32_t id) : m_name(name)
#ifdef SNAPSHOT
        , m_snapshot(nullptr)
#endif
    {
        m_parent = parent;

        //Initialize the volumetric mask
        if(parent)
        {
            size_t nbData = sizeof(uint8_t)*(parent->getNbSpatialData()+7)/8;
            m_volumetricMask = (uint8_t*)malloc(nbData);
            resetVolumetricMask(false, false);
        }
        setID(id);
    }

    SubDataset::SubDataset(const SubDataset& sd)
    {
        *this = sd;
    }

    SubDataset& SubDataset::operator=(const SubDataset& sd)
    {
        if(this != &sd)
        {
            m_isValid       = sd.m_isValid;
            m_rotation      = sd.m_rotation;
            m_position      = sd.m_position;
            m_scale         = sd.m_scale;
            m_parent        = sd.m_parent;
            m_name          = sd.m_name;
            m_depthClipping = sd.m_depthClipping;

            //TODO copy that in a better way
            m_tf        = sd.m_tf;

            for(auto& it : sd.m_annotationCanvases)
                m_annotationCanvases.push_back(std::shared_ptr<AnnotationCanvas>(new AnnotationCanvas(*it.get())));

            for(auto& it : sd.m_annotationPositions)
                m_annotationPositions.push_back(std::shared_ptr<DrawableAnnotationPosition>(new DrawableAnnotationPosition(*it.get())));

            if(m_parent)
            {
                size_t nbData = sizeof(uint8_t)*(m_parent->getNbSpatialData()+7)/8;
                m_volumetricMask = (uint8_t*)malloc(nbData);
                memcpy(m_volumetricMask, sd.m_volumetricMask, nbData);
            }
        }

        return *this;
    }

    SubDataset::~SubDataset()
    {
        if(m_volumetricMask)
            free(m_volumetricMask);
        if(m_sdGroup)
            m_sdGroup->removeSubDataset(this);
    }

    AnnotationCanvas* SubDataset::emplaceAnnotationCanvas(uint32_t w, uint32_t h, float* position)
    {
        AnnotationCanvas* annot = new AnnotationCanvas(w, h, position);
        m_annotationCanvases.push_back(std::shared_ptr<AnnotationCanvas>(annot));
        return annot;
    }

    void SubDataset::addAnnotationCanvas(std::shared_ptr<AnnotationCanvas> annot)
    {
        m_annotationCanvases.push_back(annot);
    }

    bool SubDataset::removeAnnotationCanvas(std::shared_ptr<AnnotationCanvas> annot)
    {
        for(std::list<std::shared_ptr<AnnotationCanvas>>::const_iterator it = m_annotationCanvases.begin(); it != m_annotationCanvases.end(); it++)
            if((*it) == annot)
                return removeAnnotationCanvas(it) != m_annotationCanvases.end();
        return false;
    }

    std::list<std::shared_ptr<AnnotationCanvas>>::const_iterator SubDataset::removeAnnotationCanvas(std::list<std::shared_ptr<AnnotationCanvas>>::const_iterator it)
    {
        return m_annotationCanvases.erase(it);
    }

    size_t SubDataset::getVolumetricMaskSize() const 
    {
        return sizeof(uint8_t)*(m_parent->getNbSpatialData()+7)/8;
    }

    void SubDataset::resetVolumetricMask(bool t, bool enable)
    {
        memset(m_volumetricMask, (t ? 0xff : 0x00), getVolumetricMaskSize());
        m_enableVolumetricMask = enable;
    }

    glm::mat4 SubDataset::getModelWorldMatrix() const
    {
        glm::mat4 posMat(1.0f);
        posMat = glm::translate(posMat, getPosition());
        posMat = posMat * getGlobalRotate().getMatrix();
        posMat = glm::scale(posMat, getScale());

        return posMat;
    }

    void SubDataset::setSubDatasetGroup(SubDatasetGroup* group)
    {
        if(m_sdGroup != group)
        {
            if(m_sdGroup != nullptr)
            {
                SubDatasetGroup* oldGroup = m_sdGroup;
                m_sdGroup = nullptr;
                oldGroup->removeSubDataset(this);
            }
            m_sdGroup = group;
            if(m_sdGroup)
            {
                const auto& subdatasets = m_sdGroup->getSubDatasets();
                if(std::find(subdatasets.begin(), subdatasets.end(), this) != subdatasets.end() &&
                   !m_sdGroup->addSubDataset(this))
                    m_sdGroup = nullptr;
            }
        }
    }
}
