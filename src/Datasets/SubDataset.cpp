#include "Datasets/SubDataset.h"
#include "Datasets/Dataset.h"

namespace sereno
{
    SubDataset::SubDataset(Dataset* parent, const std::string& name) :  m_amplitude{std::numeric_limits<float>::max(), std::numeric_limits<float>::min()}, m_name(name)
#ifdef SNAPSHOT
        , m_snapshot(nullptr)
#endif
    {
        m_parent = parent;
    }

    void SubDataset::setClamping(float min, float max)
    {
        m_minClamp  = min;
        m_maxClamp  = max;
    }
}
