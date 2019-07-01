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

    SubDataset::~SubDataset()
    {
    }

    void SubDataset::setClamping(float min, float max)
    {
        m_minClamp  = min;
        m_maxClamp  = max;
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
}
