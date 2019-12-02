#include "Datasets/Dataset.h"
#include <utility>

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
        return *this;
    }

    Dataset::~Dataset()
    {
        while(m_subDatasets.size() != 0)
            removeSubDataset(m_subDatasets.back());
    }

    uint32_t Dataset::getTFIndiceFromPointFieldID(uint32_t pID)
    {
        for(uint32_t i = 0; i < m_pointFieldDescs.size(); i++)
            if(m_pointFieldDescs[i].id == pID)
                return i;
        return -1;
    }
}
