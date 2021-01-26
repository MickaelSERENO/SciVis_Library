#include "Datasets/Annotation/AnnotationLogContainer.h"
#include <algorithm>

namespace sereno
{
    AnnotationLogContainer::~AnnotationLogContainer()
    {
        for(auto it : m_positions)
            it.first->removeListener(this);
    }

    std::shared_ptr<AnnotationPosition> AnnotationLogContainer::buildAnnotationPositionView() const 
    {
        return std::make_shared<AnnotationPosition>(this);
    }

    int32_t AnnotationLogContainer::parseAnnotationPosition(std::shared_ptr<AnnotationPosition> annot)
    {
        if(annot->getAnnotationLog() != this)
            return ANNOTATION_LOG_CONTAINER_INVALID_PARENT;

        //Get the indices
        int32_t indices[3];
        annot->getPosIndices(indices);
        std::sort(indices, indices+3);

        auto it    = m_assignedHeaders.begin();
        int   i    = 0;
        bool found = false;
        while(i < 3 && it != m_assignedHeaders.end())
        {
            if(indices[i] != -1)
            {
                it = std::lower_bound(it, m_assignedHeaders.end(), indices[i]);
                if(it != m_assignedHeaders.end() && indices[i] == (int32_t)(*it))
                {
                    found = true;
                    break;
                }
            }
            i++;
        } 

        if(found)
            return ANNOTATION_LOG_CONTAINER_ERROR_HEADER_ALREADY_PRESENT;

        m_positions.emplace(annot, std::vector<glm::vec3>(annot->begin(), annot->end()));

        it = m_assignedHeaders.begin();
        for(auto i : indices)
        {
            if(i != -1)
            {
                it = std::upper_bound(it, m_assignedHeaders.end(), i);
                it = m_assignedHeaders.insert(it, i);
            }
        }
        
        annot->addListener(this);
        return 0;
    }

    const std::vector<glm::vec3>* AnnotationLogContainer::getPositionsFromView(std::shared_ptr<AnnotationPosition> annot) const
    {
        auto it = m_positions.find(annot);
        if(it != m_positions.end())
            return &it->second;
        return NULL;
    }

    std::vector<uint32_t> AnnotationLogContainer::getRemainingHeaders() const
    {
        std::vector<uint32_t> res;
        uint32_t nbColumns = getNbColumns();
        res.reserve(nbColumns);
        uint32_t i = 0;

        for(uint32_t j = 0; i < nbColumns && j < m_assignedHeaders.size(); i++, j++)
        {
            for(; i < nbColumns && i != m_assignedHeaders[j]; i++) //This works because every list is ordered
                res.push_back(i);
        }

        for(; i < nbColumns; i++)
            res.push_back(i);

        return res;
    }

    void AnnotationLogContainer::onParse()
    {
        readTimeValues();
    }

    void AnnotationLogContainer::onSetTimeColumn()
    {
        readTimeValues();
    }

    void AnnotationLogContainer::readTimeValues()
    {
        //Erase old header
        if(m_curTimeHeader != -1)
        {
            auto it = std::lower_bound(m_assignedHeaders.begin(), m_assignedHeaders.end(), m_curTimeHeader);
            if(it != m_assignedHeaders.end()) //THIS SHOULD ALWAYS BE TRUE
                m_assignedHeaders.erase(it);
        }

        m_curTimeHeader = getTimeColumn();

        if(m_curTimeHeader != -1)
        {
            auto it = std::upper_bound(m_assignedHeaders.begin(), m_assignedHeaders.end(), m_curTimeHeader);
            it = m_assignedHeaders.insert(it, m_curTimeHeader);
            m_time = getTimeValues();
        }
        else
            m_time.clear();
    }

    void AnnotationLogContainer::onUpdateHeaders(AnnotationLogComponent* component, const std::vector<int32_t>& oldHeaders)
    {
        bool changeHeaders = false;

        //Erase the headers as assigned
        for(auto h : oldHeaders)
        {
            for(auto it = m_assignedHeaders.begin(); it != m_assignedHeaders.end();)
            {
                if(*it == (uint32_t)h)
                {
                    it = m_assignedHeaders.erase(it);
                    break;
                }
                it++;
            }
        }

        //Add the assigned headers. Set as "-1" if the header is already taken
        std::vector<int32_t> currentHeaders = component->getHeaders();
        for(auto& it : currentHeaders)
        {
            if(it != -1)
            {
                auto oldVal = it;
                //If found: set header as -1
                if(std::binary_search(m_assignedHeaders.begin(), m_assignedHeaders.end(), it))
                {
                    it            = -1;
                    changeHeaders = true;
                }

                //We can "still" add it (even if, for the moment, it exists twice), because this function shall be called twice
                auto insertIT = std::upper_bound(m_assignedHeaders.begin(), m_assignedHeaders.end(), oldVal);
                m_assignedHeaders.insert(insertIT, oldVal);
            }
        }

        //Update internal data. Rechange the headers if required (then stop this function)
        for(auto& it : m_positions)
        {
            if(it.first.get() == component)
            {
                if(changeHeaders)
                {
                    it.first->setXYZIndices(currentHeaders[0], currentHeaders[1], currentHeaders[2]);
                    return; //This method shall be called again due to the setXYZIndices
                }
                else
                {
                    it.second.assign(it.first->begin(), it.first->end());
                    return;
                }
            }
        }
    }
}
