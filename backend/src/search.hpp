#include <string>
#include "aminerdata.hpp"

struct SearchService {
	SearchService(std::unique_ptr<AMinerData>&& data);
	bool PubSearch(const std::string&, std::string&);
	bool PubSearchByAuthor(const std::string&, std::string&);
	bool AuthorSearch(const std::string&, std::string&);
private:
	std::unique_ptr<AMinerData> aminer;
};
