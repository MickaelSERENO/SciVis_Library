#include "Datasets/Dataset.h"
#include <utility>
#include <algorithm>

namespace sereno
{
    Dataset::Dataset(const Dataset& copy)
    {
        *this = copy;
    }

    Dataset::Dataset(Dataset&& mvt)
    {
        m_subDatasets = std::move(mvt.m_subDatasets);
    }

    Dataset& Dataset::operator=(const Dataset& copy)
    {
        if(this == &copy)
            return *this;
        m_subDatasets = std::move(copy.m_subDatasets);
        m_grads       = std::move(copy.m_grads);
        return *this;
    }

    Dataset::~Dataset()
    {
        while(m_subDatasets.size() != 0)
            removeSubDataset(m_subDatasets.back());
        for(auto it : m_grads)
            delete it;
    }

    uint32_t Dataset::getTFIndiceFromPointFieldID(uint32_t pID)
    {
        for(uint32_t i = 0; i < m_pointFieldDescs.size(); i++)
            if(m_pointFieldDescs[i].id == pID)
                return i;
        return -1;
    }


    DatasetGradient* Dataset::getOrComputeGradient(const std::vector<uint32_t>& indices)
    {
        //Search for an existing computed gradient
        std::vector<uint32_t> idsCpy = indices;
        std::sort(idsCpy.begin(), idsCpy.end());
        for(auto& it : m_grads)
        {
            if(idsCpy == it->indices)
                return it;
        }

        //Compute the gradient, store it, and return it
        DatasetGradient* grad = computeGradient(idsCpy);
        if(grad)
            m_grads.push_back(grad);
        return grad;
    }
}
