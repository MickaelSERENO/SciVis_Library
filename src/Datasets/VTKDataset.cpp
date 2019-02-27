#include "Datasets/VTKDataset.h"

namespace sereno
{
    VTKDataset::VTKDataset(std::shared_ptr<VTKParser>& parser, const std::vector<const VTKFieldValue*>& ptFieldValues, 
                           const std::vector<const VTKFieldValue*>& cellFieldValues) : m_ptFieldValues(ptFieldValues), m_cellFieldValues(cellFieldValues), m_parser(parser)
    {
        for(uint32_t i = 0; i < m_ptFieldValues.size() + m_cellFieldValues.size(); i++)
        {
            m_subDatasets.push_back(new SubDataset(this));
            setSubDatasetValidity(m_subDatasets.back(), true);
        }
    }
}
