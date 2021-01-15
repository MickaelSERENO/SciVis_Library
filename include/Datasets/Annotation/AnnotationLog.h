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
        /**
         * \brief  Search for "header" in "headers" and returns its position in the list
         *
         * \param headers the list of headers to look upon
         * \param header the header to look at
         *
         * \return   an index corresponding to the positions of headers where header was found: headers[index] == header. -1 if header was not found */
        static int32_t indiceFromHeader(const std::vector<std::string>& headers, const std::string& header);

        public:
            /** \brief  A class corresponding to a row. This object should not outlive the AnnotationLog it is attached to */
            class LogEntry
            {
                public:
                    /** \brief  Constructor
                     *
                     * \param values the values of this row log
                     * \param header a pointer to the header if the header exists. NULL otherwise
                     */
                    LogEntry(const std::vector<std::string>& values, const std::vector<std::string>* header=NULL) : m_values(values), m_header(header) {}

                    /** \brief  Move Constructor
                     *
                     * \param values the values of this row log
                     * \param header a pointer to the header if the header exists. NULL otherwise */
                    LogEntry(std::vector<std::string>&& values, const std::vector<std::string>* header=NULL) : m_values(values), m_header(header) {}

                    /** \brief  Get the number of stored values
                     * \return  The number of stored values */
                    size_t size()                          const {return m_values.size();}


                    /** \brief  Is this row associated to a header?
                     * \return   true if yes, false otherwise */
                    bool   hasHeader()                     const {return m_header;}

                    /** \brief  Does the header "h" exist?
                     * \return   true if yes, false otherwise */
                    bool   hasHeader(const std::string& h) const {return hasHeader() && AnnotationLog::indiceFromHeader(*m_header, h) >= 0;}

                    /** \brief  Access the string value at "index". hasHeader(index) should return true, or this function has an undefined behavior.
                     * \param index the header to look after
                     * \return   a const reference string value corresponding at the position "index" as defined by the header*/
                    const std::string& operator[](const std::string& index) const;

                    /** \brief  Access the string value at "index". hasHeader(index) should return true, or this function has an undefined behavior.
                     * \param index the header to look after
                     * \return   a reference string value corresponding at the position "index" as defined by the header*/
                    std::string& operator[](const std::string& index) {return const_cast<std::string&>(std::as_const(*this)[index]);}

                    /** \brief Access the string value at index "i". 
                     * \param i the column position to look after
                     * \return a const reference string value corresponding at the position "i" in the row*/
                    const std::string& operator[](uint32_t i) const;

                    /** \brief Access the string value at index "i". 
                     * \param i the column position to look after
                     * \return a reference string value corresponding at the position "i" in the row*/
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
            /** \brief  Initialize the Log reading
             *
             * \param header should we expect a header when reading data?  */
            AnnotationLog(bool header=true);

            /** \brief  Destructor */
            virtual ~AnnotationLog();

            /** \brief  Is this data associated to a header?
             * \return   true if yes, false otherwise */
            bool   hasHeader() const {return m_hasHeader;}

            /** \brief  Does the header "h" exist?
             * \return   true if yes, false otherwise */
            bool   hasHeader(const std::string& h) const {return hasHeader() && AnnotationLog::indiceFromHeader(m_header, h) >= 0;}

            /** \brief  The number of stored rows
             * \return  The number of rows */
            uint32_t size() {return m_values.size();}

            /** \brief  Access the i-th row
             * \param i the row indice to look after
             * \return   a const reference to a LogEntry representing the i-th row*/
            const LogEntry& operator[](uint32_t i) const {return m_values[i];}

            /** \brief  Access the i-th row
             * \param i the row indice to look after
             * \return   a reference to a LogEntry representing the i-th row*/
            LogEntry&       operator[](uint32_t i)       {return m_values[i];}

            /** \brief  Try to find the column indice corresponding to the header h
             * \param h the header to look after
             * \return   -1 if not found, the indice of the "h" values otherwise */
            int32_t indiceFromHeader(const std::string& h) const {return m_hasHeader?AnnotationLog::indiceFromHeader(m_header, h):-1;}

            /** \brief Initialize the logs from a csv file
             * \param path the file to read
             * \return   true if success, false otherwise. A message is printed in std::cerr in case of errors */
            bool readFromCSV(const std::string& path);

            /** \brief  Set the column indice where time is expected. Negative values == no expected time
             * \param timeCol the time column indice. Negatif values for no expected time
             * \return   return false in case of an invalid time column: No values were entered (and timeCol is positive), or timeCol is outside the number of columns of this annotation.  */
            bool setTimeColumn(int32_t timeCol);

            /** \brief  Set the column indice where time is expected.
             * \param timeHeader the header column corresponding to the time values. Must be a valid header (see hasHeader())
             * \return   return false in case of an invalid time header column*/
            bool setTimeColumn(const std::string& timeHeader);

            /** \brief  Get the column indice associated with time data
             * \return   -1 if no time is available, the indice corresponding to time data otherwise */
            int32_t getTimeColumn() const {return m_timeIT;}

            std::vector<LogEntry>::iterator begin() {return m_values.begin();}
            std::vector<LogEntry>::iterator end()   {return m_values.end();}

            std::vector<LogEntry>::const_iterator begin() const {return m_values.begin();}
            std::vector<LogEntry>::const_iterator end()   const {return m_values.end();}
        private:
            bool m_hasHeader;
            bool m_hasRead = false;
            std::vector<std::string> m_header;
            std::vector<LogEntry>    m_values;
            int32_t                  m_timeIT = -1;
    };
}

#endif
