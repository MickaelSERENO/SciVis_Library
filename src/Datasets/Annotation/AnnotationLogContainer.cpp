#include "Datasets/Annotation/AnnotationLogContainer.h"
#include <algorithm>

namespace sereno
{
    AnnotationLogContainer::~AnnotationLogContainer()
    {
        for(auto it : m_positions)
            delete it.second;
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

        m_positions.emplace(annot, new std::vector<glm::vec3>(annot->begin(), annot->end()));

        it = m_assignedHeaders.begin();
        for(auto i : indices)
        {
            if(i != -1)
            {
                it = std::upper_bound(it, m_assignedHeaders.end(), i);
                it = m_assignedHeaders.insert(it, i);
            }
        }
        
        return 0;
    }

    const std::vector<glm::vec3>* AnnotationLogContainer::getPositionsFromView(std::shared_ptr<AnnotationPosition> annot) const
    {
        auto it = m_positions.find(annot);
        if(it != m_positions.end())
            return it->second;
        return NULL;
    }

    std::vector<uint32_t> AnnotationLogContainer::getRemainingHeaders() const
    {
        std::vector<uint32_t> res;
        for(uint32_t i = 0, j = 0; i < size(); i++)
        {
            for(; i < size() && i != m_assignedHeaders[j]; i++)
                res.push_back(i);
            j++;
        }

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
}