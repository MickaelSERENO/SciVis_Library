#include "Datasets/VTKDataset.h"

namespace sereno
{
    VTKDataset::VTKDataset(std::shared_ptr<VTKParser>& parser, const std::vector<const VTKFieldValue*>& ptFieldValues, 
                           const std::vector<const VTKFieldValue*>& cellFieldValues) : m_ptFieldValues(ptFieldValues), m_cellFieldValues(cellFieldValues), m_parser(parser)
    {
        for(auto v : m_ptFieldValues)
        {
            m_subDatasets.push_back(new SubDataset(this, v->name));
            setSubDatasetValidity(m_subDatasets.back(), true);
        }

        for(auto v : m_cellFieldValues)
        {
            m_subDatasets.push_back(new SubDataset(this, v->name));
            setSubDatasetValidity(m_subDatasets.back(), true);
        }
    }
}
