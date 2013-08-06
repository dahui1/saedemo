#include <string>
#include "aminerdata.hpp"

struct SearchService {
    SearchService(std::unique_ptr<AMinerData>&& data);
    bool PubSearch(const std::string&, std::string&);
    bool PubSearchByAuthor(const std::string&, std::string&);
    bool AuthorSearch(const std::string&, std::string&);
    bool AuthorSearchById(const string& input, string& output);
    bool InfluenceSearchByAuthor(const string& input, string& output);
private:
    std::unique_ptr<AMinerData> aminer;
};
