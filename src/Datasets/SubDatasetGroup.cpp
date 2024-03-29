#include "Datasets/SubDatasetGroup.h"
#include <cstring>

namespace sereno
{
    SubDatasetGroup::~SubDatasetGroup()
    {
        for(auto& sd : m_subDatasets)
            sd->setSubDatasetGroup(nullptr);
    }

    bool SubDatasetGroup::removeSubDataset(SubDataset* sd)
    {
        auto _sd = std::find(m_subDatasets.begin(), m_subDatasets.end(), sd);
        if(_sd != m_subDatasets.end())
        {
            m_subDatasets.erase(_sd);
            sd->setSubDatasetGroup(nullptr);
            return true;
        }
        return false;
    }

    bool SubDatasetGroup::addSubDataset(SubDataset* sd)
    {
        auto _sd = std::find(m_subDatasets.begin(), m_subDatasets.end(), sd);
        if(_sd == m_subDatasets.end())
        {
            m_subDatasets.push_back(sd);
            sd->setSubDatasetGroup(this);
            return true;
        }
        return false;
    }

    SubDatasetSubjectiveGroup::SubDatasetSubjectiveGroup(SubDataset* base): SubDatasetGroup(), m_base(base)
    {
        addSubDataset(m_base);
    }

    SubDatasetSubjectiveGroup::~SubDatasetSubjectiveGroup(){}

    SubDatasetSubjectiveStackedGroup::SubDatasetSubjectiveStackedGroup(SubDataset* base) : SubDatasetSubjectiveGroup(base)
    {
    }

    SubDatasetSubjectiveStackedGroup::~SubDatasetSubjectiveStackedGroup(){}

    SubDatasetSubjectiveStackedLinkedGroup::SubDatasetSubjectiveStackedLinkedGroup(SubDataset* base) : SubDatasetSubjectiveStackedGroup(base)
    {}

    void SubDatasetSubjectiveStackedLinkedGroup::updateSubDatasets()
    {
        if(m_base == nullptr)
            return;

        const glm::vec3&   scale = m_base->getScale();
        const glm::vec3&   pos   = m_base->getPosition();
        const Quaternionf& rot   = m_base->getGlobalRotate();

        float size = scale.x*scale.x + scale.y*scale.y + scale.z*scale.z;
        size = sqrt(size);

        for(auto& it : m_subjViews)
        {
            if(it.first)
            {
                it.first->setScale(scale);
                it.first->setGlobalRotate(rot);

                if(it.second != nullptr)
                {
                    it.first->enableVolumetricMask(it.second->isVolumetricMaskEnabled());
                    it.first->setDepthClipping(it.second->getMinDepthClipping(), it.second->getMaxDepthClipping());
                    if(it.second->getTransferFunction() != nullptr)
                        it.first->setTransferFunction(std::shared_ptr<TF>(it.second->getTransferFunction()->clone()));
                    else
                        it.first->setTransferFunction(nullptr);

                    if(it.second->isVolumetricMaskEnabled() && it.second->getVolumetricMaskSize() == it.first->getVolumetricMaskSize())
                    {
                        memcpy(it.first->getVolumetricMask(), it.second->getVolumetricMask(), it.second->getVolumetricMaskSize());
                    }
                }
            }
        }

        if(m_stack == STACK_VERTICAL)
        {
            uint32_t i = 0;
            for(auto& it : m_subjViews)
            {
                if(it.first)
                {
                    it.first->setPosition(glm::vec3(pos.x, pos.y+(i+1)*(m_gap+size), pos.z));
                    if(!m_mergeSubjViews)
                        i++;
                }
            }
        }
        else if(m_stack == STACK_HORIZONTAL)
        {
            uint32_t i = 0;
            for(auto& it : m_subjViews)
            {
                if(it.first)
                {
                    it.first->setPosition(glm::vec3(pos.x+(i+1)*(m_gap+size), pos.y, pos.z));
                    if(!m_mergeSubjViews)
                        i++;
                }
            }
        }
    }

    bool SubDatasetSubjectiveStackedLinkedGroup::removeSubDataset(SubDataset* sd)
    {
        if(sd == nullptr)
            return false;

        //If remove the base --> remove everything from this group
        if(sd == m_base)
        {
            SubDataset* _base = m_base;
            m_base = nullptr;
            SubDatasetGroup::removeSubDataset(_base);
            for(auto it = m_subjViews.begin(); it != m_subjViews.end();)
            {
                SubDataset* sd1 = it->first; //Save and remove BEFORE calling function because of domino effect between SubDataset and SubDatasetGroup
                SubDataset* sd2 = it->second;
                it = m_subjViews.erase(it);
                if(sd1)
                    SubDatasetGroup::removeSubDataset(sd1);
                if(sd2)
                    SubDatasetGroup::removeSubDataset(sd2);
            }
            return true;
        }

        //Otherwise, test if we are removing a subjective view
        for(auto it = m_subjViews.begin(); it != m_subjViews.end(); it++)
        {
            //If yes, remove both the linked and the stacked subjective views
            if(it->second == sd || it->first == sd)
            {
                SubDataset* sd1 = it->first; //Save and remove BEFORE calling function because of domino effect between SubDataset and SubDatasetGroup
                SubDataset* sd2 = it->second;
                m_subjViews.erase(it);
                if(sd1)
                    SubDatasetGroup::removeSubDataset(sd1);
                if(sd2)
                    SubDatasetGroup::removeSubDataset(sd2);
                return true;
            }
        }

        return false;
    }

    bool SubDatasetSubjectiveStackedLinkedGroup::addSubjectiveSubDataset(SubDataset* sdStacked, SubDataset* sdLinked)
    {
        if(sdStacked == nullptr && sdLinked == nullptr)
            return false;

        //Test if those are already registered
        for(auto it : m_subDatasets)
            if(it == sdStacked || it == sdLinked)
                return false;

        if(sdStacked)
            addSubDataset(sdStacked);
        if(sdLinked)
            addSubDataset(sdLinked);

        m_subjViews.push_back(std::make_pair(sdStacked, sdLinked));
        return true;
    }

    bool SubDatasetSubjectiveStackedLinkedGroup::isSpatiallyModifiable(SubDataset* sd)
    {
        for(auto& it : m_subjViews)
            if(it.first == sd)
                return false;
        return true;
    }

    std::pair<SubDataset*, SubDataset*> SubDatasetSubjectiveStackedLinkedGroup::getLinkedSubDataset(SubDataset* sd)
    {
        if(sd == nullptr)
            return std::pair(nullptr, nullptr);

        for(auto& it : m_subjViews)
            if(it.first == sd || it.second == sd)
            {
                if(it.first == nullptr)
                {
                    auto cpy = it;
                    cpy.first = getBase();
                    return cpy;
                }
                return it;
            }
        return std::make_pair(nullptr, nullptr);
    }

    std::pair<const SubDataset*, const SubDataset*> SubDatasetSubjectiveStackedLinkedGroup::getLinkedSubDataset(SubDataset* sd) const
    {
        if(sd == nullptr)
            return std::pair(nullptr, nullptr);

        for(auto& it : m_subjViews)
            if(it.first == sd || it.second == sd)
            {
                if(it.first == nullptr)
                {
                    std::pair<const SubDataset*, const SubDataset*> cpy = it;
                    cpy.first = getBase();
                    return cpy;
                }
                return it;
            }
        return std::make_pair(nullptr, nullptr);
    }
}
