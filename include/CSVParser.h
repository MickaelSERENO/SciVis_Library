#ifndef  CSVPARSER_INC 
#define  CSVPARSER_INC

#include <iterator>
#include <cstddef>
#include <vector>
#include <iostream>
#include <string>

//This code was adapted from https://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c

/** \brief  Read the tokens of the next CSV lines. This code handles escaped strings
 *
 * \param str the input stream to read from
 * \param separator the separator to use for the CSV input stream
 *
 * \return  the list of tokens as a string */
std::vector<std::string> getCSVLineTokens(std::istream& str, char separator=',');


/** \brief  Represent a CSV row */
class CSVRow
{
    public:
        /** \brief  Constructor, initialize the reader
         *
         * \param separator the separator to use. Default: comma */
        CSVRow(char separator=',') : m_separator(separator) {}

        /** \brief  Access the token at indice "index"
         *
         * \param index the token to access
         *
         * \return  a non-const reference to the token */
        std::string& operator[](size_t index)
        {
            return m_data[index];
        }

        /** \brief  Access the token at indice "index"
         *
         * \param index the token to access
         *
         * \return  a const-reference to the token */
        const std::string& operator[](size_t index) const
        {
            return m_data[index];
        }

        /** \brief  Access the number of tokens currently stored in the current row
         *
         * \return  the number of tokens for this row */
        std::size_t size() const
        {
            return m_data.size();
        }

        /** \brief  Read the next row of the stream "str". Tokens are accessible using the [] operator
         *
         * \param str the stream to read from */
        void readNextRow(std::istream& str)
        {
            m_data = getCSVLineTokens(str, m_separator);
        }

        std::vector<std::string>::iterator begin() {return m_data.begin();}
        std::vector<std::string>::iterator end()   {return m_data.end();}

        std::vector<std::string>::const_iterator begin() const {return m_data.begin();}
        std::vector<std::string>::const_iterator end()   const {return m_data.end();}

        const std::vector<std::string>& getData() const {return m_data;}
    private:
        char m_separator;
        std::vector<std::string> m_data;
};

/** \brief  >> operator to read the next row in str
 *
 * \param str the stream to read from
 * \param data the CSV Row to store the data in
 *
 * \return  a reference to str */
inline std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}   

/** \brief  Range iterator class */
class CSVRange
{
    public:
        class CSVIterator
        {   
            public:
                //Iterator specifications
                typedef std::input_iterator_tag     iterator_category;
                typedef ptrdiff_t                   difference_type;
                typedef CSVRow                      value_type;
                typedef CSVRow*                     pointer;
                typedef CSVRow&                     reference;

                CSVIterator(std::istream& str, char separator=',') :m_str(str.good() ? &str:NULL), m_row(separator) { ++(*this); } //Read the first row if possible
                CSVIterator()                                      :m_str(NULL) {}

                // Pre Increment
                CSVIterator& operator++()               {if(m_str) {if(!((*m_str) >> m_row)){m_str = NULL;}}return *this;}
                // Post increment
                CSVIterator operator++(int)             {CSVIterator    tmp(*this);++(*this);return tmp;} //Return current, increment
                CSVRow const& operator*()   const       {return m_row;}
                CSVRow const* operator->()  const       {return &m_row;}

                bool operator==(CSVIterator const& rhs) {return ((this == &rhs) || ((this->m_str == NULL) && (rhs.m_str == NULL)));}
                bool operator!=(CSVIterator const& rhs) {return !((*this) == rhs);}
            private:
                std::istream*       m_str;
                CSVRow              m_row;
        };
    public:
        CSVRange(std::istream& str, char separator=',') : m_stream(str), m_separator(separator) {}
        CSVIterator begin() const {return CSVIterator{m_stream, m_separator};}
        CSVIterator end()   const {return CSVIterator{};}
    private:
        std::istream& m_stream;
        char m_separator;
};

#endif
