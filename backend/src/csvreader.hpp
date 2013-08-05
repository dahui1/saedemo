#pragma once
#include <string>
#include <iostream>
#include <vector>

/*
 * A naive csv reader for the Excel dialect.
 */
struct CSVReader {
    CSVReader(std::istream& source);
    std::istream& readrow(std::vector<std::string>& line);

private:
    std::istream& source;
    bool in_quoted_field = false;
    bool in_quote = false;
    bool new_line = false;
};
