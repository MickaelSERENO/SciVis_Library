#include "CSVParser.h"

std::vector<std::string> getCSVLineTokens(std::istream& str, char separator)
{
    std::vector<std::string>   result;
    std::string cell;
    bool inStr         = false;
    bool inSecondQuote = false;
    char c;

    while(str.get(c))
    {
        switch(c)
        {
            case '"':
                if(inStr)
                {
                    if(inSecondQuote)
                    {
                        cell += '"';
                        inSecondQuote = false;
                    }
                    else
                        inSecondQuote = true;
                }
                else
                    inStr = true;
                break;
            default:
                if(inSecondQuote)
                {
                    inStr = false;
                    inSecondQuote = false;
                }

                if(inStr)
                {
                    cell += c;
                }

                else if(c == separator)
                {
                    result.push_back(cell);
                    cell = "";
                }
                else if(c == '\n')
                {
                    result.push_back(cell);
                    return result;
                }
                else if(c != '\r') //remove carriage return
                    cell += c;
                break;
        }
    }

    if(result.size() || !cell.empty())
        result.push_back(cell);
    return result;
}

