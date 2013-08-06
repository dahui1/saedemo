#include "csvreader.hpp"

CSVReader::CSVReader(std::istream& source) : source(source) {
}

std::istream& CSVReader::readrow(std::vector<std::string>& line) {
    std::string field;
    line.clear();

    char c;
    while (source.get(c)) {
        switch (c) {
            case '"':
                new_line = false;
                if (in_quoted_field && !in_quote) {
                    field += c;
                }
                in_quote = !in_quote;
                in_quoted_field = true;
                break;

            case ',':
                new_line = false;
                if (in_quote == true)
                {
                    field += c;
                }
                else
                {
                    line.push_back(field);
                    field.clear();
                    in_quoted_field = false;
                }
                break;

            case '\n':
            case '\r':
                if (in_quote == true)
                {
                    field += c;
                }
                else
                {
                    if (new_line == false)
                    {
                        line.push_back(field);

                        new_line = true;
                        in_quoted_field = false;
                        return source;
                    }
                }
                break;

            default:
                new_line = false;
                field.push_back(c);
                break;
        }
    }
    line.push_back(field);
    return source;
}
