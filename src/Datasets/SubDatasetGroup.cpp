#include "Datasets/SubDatasetGroup.h"

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
        if(_sd != m_subDatasets.end())
        {
            m_subDatasets.push_back(sd);
            sd->setSubDatasetGroup(this);
            return true;
        }
        return false;
    }

    SubDatasetSubjectiveGroup::SubDatasetSubjectiveGroup(SubDataset* base) : SubDatasetGroup(), m_base(base)
    {
        addSubDataset(base);
    }

    SubDatasetSubjectiveGroup::~SubDatasetSubjectiveGroup(){}

    SubDatasetSubjectiveStackedGroup::SubDatasetSubjectiveStackedGroup(SubDataset* base) : SubDatasetSubjectiveGroup(base)
    {}

    void SubDatasetSubjectiveStackedGroup::updateSubDatasets()
    {
        if(m_base == nullptr)
            return;

        const glm::vec3&   scale = m_base->getScale();
        const glm::vec3&   pos   = m_base->getPosition();
        const Quaternionf& rot   = m_base->getGlobalRotate();

        float size = scale.x*scale.x + scale.y*scale.y + scale.z*scale.z;
        size = sqrt(size);

        for(auto it : m_subjViews)
        {
            it->setScale(scale);
            it->setGlobalRotate(rot);
        }

        if(m_stack == STACK_VERTICAL)
        {
            uint32_t i = 0;
            for(auto it : m_subjViews)
            {
                it->setPosition(glm::vec3(pos.x, pos.y+(i+1)*(m_gap+size), pos.z));
                if(!m_mergeSubjViews)
                    i++;
            }
        }
        else if(m_stack == STACK_HORIZONTAL)
        {
            uint32_t i = 0;
            for(auto it : m_subjViews)
            {
                it->setPosition(glm::vec3(pos.x+(i+1)*(m_gap+size), pos.y, pos.z));
                if(!m_mergeSubjViews)
                    i++;
            }
        }
    }

    bool SubDatasetSubjectiveStackedGroup::addSubjectiveSubDataset(SubDataset* sd)
    {
        if(!m_base || ! addSubDataset(sd))
            return false;
        m_subjViews.push_back(sd);
        return true;
    }

    bool SubDatasetSubjectiveStackedGroup::removeSubDataset(SubDataset* sd)
    {
        //If remove the base --> remove everything from this group
        if(sd == m_base)
        {
            m_base = nullptr;
            SubDatasetGroup::removeSubDataset(m_base);
            for(auto it : m_subjViews)
                SubDatasetGroup::removeSubDataset(it);
            m_subjViews.clear();
            return true;
        }

        //Otherwise, test if we are removing a subjective view
        auto _sd = std::find(m_subjViews.begin(), m_subjViews.end(), sd);
        if(_sd == m_subjViews.end())
            return false;
        m_subjViews.erase(_sd);
        SubDatasetGroup::removeSubDataset(sd);
        return true;
    }

    bool SubDatasetSubjectiveStackedGroup::isSpatiallyModifiable(SubDataset* sd)
    {
        auto _sd = std::find(m_subjViews.begin(), m_subjViews.end(), sd);
        if(_sd == m_subjViews.end())
            return false;
        return true;
    }

    SubDatasetSubjectiveLinkedGroup::SubDatasetSubjectiveLinkedGroup(SubDataset* base) : SubDatasetSubjectiveGroup(base)
    {}

    void SubDatasetSubjectiveLinkedGroup::updateSubDatasets()
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
            it.first->setScale(scale);
            it.first->setGlobalRotate(rot);
        }

        if(m_stack == STACK_VERTICAL)
        {
            uint32_t i = 0;
            for(auto& it : m_subjViews)
            {
                it.first->setPosition(glm::vec3(pos.x, pos.y+(i+1)*(m_gap+size), pos.z));
                if(!m_mergeSubjViews)
                    i++;
            }
        }
        else if(m_stack == STACK_HORIZONTAL)
        {
            uint32_t i = 0;
            for(auto& it : m_subjViews)
            {
                it.first->setPosition(glm::vec3(pos.x+(i+1)*(m_gap+size), pos.y, pos.z));
                if(!m_mergeSubjViews)
                    i++;
            }
        }
    }

    bool SubDatasetSubjectiveLinkedGroup::removeSubDataset(SubDataset* sd)
    {
        //If remove the base --> remove everything from this group
        if(sd == m_base)
        {
            m_base = nullptr;
            SubDatasetGroup::removeSubDataset(m_base);
            for(auto& it : m_subjViews)
            {
                SubDatasetGroup::removeSubDataset(it.first);
                SubDatasetGroup::removeSubDataset(it.second);
            }
            m_subjViews.clear();
            return true;
        }

        //Otherwise, test if we are removing a subjective view
        for(auto it = m_subjViews.begin(); it != m_subjViews.end(); it++)
        {
            //If yes, remove both the linked and the stacked subjective views
            if(it->second == sd || it->first == sd)
            {
                SubDatasetGroup::removeSubDataset(it->first);
                SubDatasetGroup::removeSubDataset(it->second);
                m_subjViews.erase(it);
                return true;
            }
        }

        return false;
    }

    bool SubDatasetSubjectiveLinkedGroup::addSubjectiveSubDataset(SubDataset* sdStacked, SubDataset* sdLinked)
    {
        //Test if those are already registered
        for(auto it : m_subDatasets)
            if(it == sdStacked || it == sdLinked)
                return false;

        addSubDataset(sdStacked);
        addSubDataset(sdLinked);

        m_subjViews.push_back(std::make_pair(sdStacked, sdLinked));
        return true;
    }

    bool SubDatasetSubjectiveLinkedGroup::isSpatiallyModifiable(SubDataset* sd)
    {
        for(auto& it : m_subjViews)
            if(it.first == sd)
                return false;
        return true;
    }

    std::pair<SubDataset*, SubDataset*> SubDatasetSubjectiveLinkedGroup::getLinkedSubDataset(SubDataset* sd)
    {
        for(auto& it : m_subjViews)
            if(it.first == sd || it.second == sd)
                return it;
        return std::make_pair(nullptr, nullptr);
    }

    std::pair<const SubDataset*, const SubDataset*> SubDatasetSubjectiveLinkedGroup::getLinkedSubDataset(SubDataset* sd) const
    {
        for(auto& it : m_subjViews)
            if(it.first == sd || it.second == sd)
                return it;
        return std::make_pair(nullptr, nullptr);
    }
}
