#include "Datasets/SubDataset.h"
#include "Datasets/Dataset.h"

namespace sereno
{
    SubDataset::SubDataset(Dataset* parent, const std::string& name) :  m_amplitude{std::numeric_limits<float>::max(), std::numeric_limits<float>::min()}, m_name(name)
    {
        m_parent = parent;
    }

    void SubDataset::setColor(float min, float max, ColorMode mode)
    {
        m_colorMode = mode;
        m_minClamp  = min;
        m_maxClamp  = max;
    }
}
