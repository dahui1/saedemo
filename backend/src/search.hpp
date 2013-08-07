#include <string>
#include "aminerdata.hpp"

struct SearchService {
    SearchService(std::unique_ptr<AMinerData>&& adata, std::unique_ptr<PMinerData>&& pdata, std::unique_ptr<WeiboData>&& wdata);
    
    //aminer services
    bool PubSearch(const std::string&, std::string&);
    bool PubSearchByAuthor(const std::string&, std::string&);
    bool AuthorSearch(const std::string&, std::string&);
    bool AuthorSearchById(const string& input, string& output);
    bool InfluenceSearchByAuthor(const string& input, string& output);

    //pminer services
    bool PatentSearch(const std::string&, std::string&);
    bool GroupSearch(const std::string&, std::string&);
	
    //weibo services
    bool WeiboSearch(const std::string&, std::string&);
    bool UserSearch(const std::string&, std::string&);

private:
    std::unique_ptr<AMinerData> aminer;
    std::unique_ptr<PMinerData> pminer;
	std::unique_ptr<WeiboData> weibo;
};
