#include "Datasets/SubDataset.h"
#include "Datasets/Dataset.h"
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
            memset(m_volumetricMask, 0xff, nbData);
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
            m_isValid   = sd.m_isValid;
            m_rotation  = sd.m_rotation;
            m_position  = sd.m_position;
            m_scale     = sd.m_scale;
            m_parent    = sd.m_parent;
            m_name      = sd.m_name;

            //TODO copy that in a better way
            m_tf        = sd.m_tf;

            for(auto& it : sd.m_annotations)
                m_annotations.push_back(std::shared_ptr<Annotation>(new Annotation(*it.get())));

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
    }

    Annotation* SubDataset::emplaceAnnotation(uint32_t w, uint32_t h, float* position)
    {
        Annotation* annot = new Annotation(w, h, position);
        m_annotations.push_back(std::shared_ptr<Annotation>(annot));
        return annot;
    }

    void SubDataset::addAnnotation(std::shared_ptr<Annotation> annot)
    {
        m_annotations.push_back(annot);
    }

    bool SubDataset::removeAnnotation(std::shared_ptr<Annotation> annot)
    {
        for(std::list<std::shared_ptr<Annotation>>::const_iterator it = m_annotations.begin(); it != m_annotations.end(); it++)
            if((*it) == annot)
                return removeAnnotation(it) != m_annotations.end();
        return false;
    }

    std::list<std::shared_ptr<Annotation>>::const_iterator SubDataset::removeAnnotation(std::list<std::shared_ptr<Annotation>>::const_iterator it)
    {
        return m_annotations.erase(it);
    }

    size_t SubDataset::getVolumetricMaskSize() const 
    {
        return sizeof(uint8_t)*(m_parent->getNbSpatialData()+7)/8;
    }

    void SubDataset::resetVolumetricMask(bool t, bool isReset)
    {
        memset(m_volumetricMask, (t ? 0xff : 0x00), getVolumetricMaskSize());
        m_noSelection = isReset;
    }

    glm::mat4 SubDataset::getModelWorldMatrix() const
    {
        glm::mat4 posMat(1.0f);
        posMat = glm::translate(posMat, getPosition());
        posMat = posMat * getGlobalRotate().getMatrix();
        posMat = glm::scale(posMat, getScale());

        return posMat;
    }
}
