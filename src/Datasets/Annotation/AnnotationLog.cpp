#include "Datasets/Annotation/AnnotationLog.h"
#include <fstream>

namespace sereno
{
    int32_t AnnotationLog::indiceFromHeader(const std::vector<std::string>& headers, const std::string& header)
    {
        for(uint32_t i = 0; i < headers.size(); i++)
            if(headers[i] == header)
                return i;
        return -1;
    }

    AnnotationLog::AnnotationLog(bool header) : m_hasHeader(header)
    {}

    AnnotationLog::~AnnotationLog() 
    {}

    uint32_t AnnotationLog::getNbColumns() const
    {
        //Works because we are in a "data frame", i.e., rectangular data (each row possesses the same number of columns. This is checked at parsing)
        return (m_values.size() ? m_values[0].size() : 0);
    }

    bool AnnotationLog::readFromCSV(const std::string& path)
    {
        std::ifstream file(path);
        if(!file.good())
        {
            std::cerr << "Could not open the file " << path << std::endl; 
            return false;
        }

        int32_t size = -1;

        //Clear values
        m_header.clear();
        m_values.clear();

        //Read header
        if(m_hasHeader)
        {
            CSVRow row;
            file >> row;
            m_header = row.getData();
            size = m_header.size();
        }

        //Read values
        for(auto& it : CSVRange(file))
        {
            if(size != -1)
            {
                if((int32_t)it.size() != size)
                {
                    std::cerr << "Error in file " << path << " : Different column numbers per row\n";
                    return false;
                }
            }
            else
                size = it.size();
            m_values.emplace_back(std::move(it.getData()), (m_hasHeader?&m_header:NULL));
        }

        m_timeIT  = 0;
        m_hasRead = true;
        onParse();

        return true;
    }

    bool AnnotationLog::setTimeColumn(int32_t timeCol)
    {
        if(timeCol < 0)
        {
            m_timeIT = timeCol;
            return true;
        }

        if(!m_hasRead)
            return false;

        if(m_hasHeader)
        {
            if(timeCol >= (int32_t)m_header.size())
                return false;
            m_timeIT = timeCol;
        }
        else
        {
            if(m_values.size())
            {
                if(timeCol >= (int32_t)m_values[0].size())
                    return false;
                m_timeIT = timeCol;
            }
            else
                m_timeIT = timeCol;
        }
        onSetTimeColumn();
        return true;
    }

    bool AnnotationLog::setTimeColumn(const std::string& timeHeader)
    {
        if(!m_hasRead || !m_hasHeader)
            return false;

        int32_t timeCol = indiceFromHeader(m_header, timeHeader);

        if(timeCol == -1)
            return false;

        m_timeIT = timeCol;
        onSetTimeColumn();
        return true;
    }

    const std::string& AnnotationLog::LogEntry::operator[](const std::string& index) const
    {
       return (*this)[AnnotationLog::indiceFromHeader(*m_header, index)];
    }

    const std::string& AnnotationLog::LogEntry::operator[](uint32_t i) const
    {
        return m_values[i];
    }

    std::vector<float> AnnotationLog::getTimeValues() const
    {
        std::vector<float> res;
        if(m_timeIT < 0)
            return res;

        res.reserve(size());
        for(const auto& it : *this)
            res.push_back(std::stof(it[m_timeIT]));
        return res;
    }
}
