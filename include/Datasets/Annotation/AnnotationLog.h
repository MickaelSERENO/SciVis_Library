#ifndef  LOG_INC
#define  LOG_INC

#include <vector>
#include <string>
#include <utility>
#include <CSVParser.h>

namespace sereno
{
    /** \brief  Basic class storing annotations from log. This has to be combined with AnnotationPosition for example to be useful */
    class AnnotationLog
    {
        static int32_t indiceFromHeader(const std::vector<std::string>& headers, const std::string& header);

        public:
            class LogEntry
            {
                public:
                    LogEntry(const std::vector<std::string>& values, const std::vector<std::string>* header=NULL) : m_values(values), m_header(header) {}
                    LogEntry(std::vector<std::string>&& values, const std::vector<std::string>* header=NULL) : m_values(values), m_header(header) {}

                    size_t size()      const {return m_values.size();}
                    bool   hasHeader() const {return m_header;}

                    const std::string& operator[](const std::string& index) const;
                    std::string& operator[](const std::string& index) {return const_cast<std::string&>(std::as_const(*this)[index]);}

                    const std::string& operator[](uint32_t i) const;
                    std::string& operator[](uint32_t i) {return const_cast<std::string&>(std::as_const(*this)[i]);}

                    std::vector<std::string>::iterator begin() {return m_values.begin();}
                    std::vector<std::string>::iterator end()   {return m_values.end();}

                    std::vector<std::string>::const_iterator begin() const {return m_values.begin();}
                    std::vector<std::string>::const_iterator end()   const {return m_values.end();}
                private:
                    std::vector<std::string>        m_values;
                    const std::vector<std::string>* m_header;
            };
        public:
            AnnotationLog(bool header=true);
            virtual ~AnnotationLog();

            bool readFromCSV(const std::string& path);
            bool setTimeColumn(uint32_t timeCol);
            bool setTimeColumn(const std::string& timeHeader);

            std::vector<LogEntry>::iterator begin() {return m_values.begin();}
            std::vector<LogEntry>::iterator end()   {return m_values.end();}

            std::vector<LogEntry>::const_iterator begin() const {return m_values.begin();}
            std::vector<LogEntry>::const_iterator end()   const {return m_values.end();}
        private:
            bool m_hasHeader;
            bool m_hasRead = false;
            std::vector<std::string> m_header;
            std::vector<LogEntry>    m_values;
            uint32_t                 m_timeIT;
    };
}

#endif
