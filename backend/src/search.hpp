#include <string>
#include "aminerdata.hpp"

struct SearchService {
	SearchService(AMinerData& data);
	bool PubSearch(const std::string&, std::string&);
	bool AuthorSearch(const std::string&, std::string&);
	bool AuthorPublicationSearch(const std::string&, std::string&);
private:
	AMinerData& aminer;
};
