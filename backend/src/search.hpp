#include <string>
#include "aminerdata.hpp"
#include "pminerdata.hpp"
#include "weibodata.hpp"

struct SearchService {
    SearchService(std::unique_ptr<AMinerData>&& adata, std::unique_ptr<PMinerData>&& pdata, std::unique_ptr<WeiboData>&& wdata);

    //aminer services
    bool PubSearch(const std::string&, std::string&);
    bool PubSearchByAuthor(const std::string&, std::string&);
    bool AuthorSearch(const std::string&, std::string&);
    bool AuthorSearchById(const string& input, string& output);
    bool InfluenceSearchByAuthor(const string& input, string& output);
	bool JConfSearch(const string& input, string& output);

    //pminer services
    bool PatentSearch(const std::string&, std::string&);
    bool PatentSearchByInventor(const std::string&, std::string&);
    bool PatentSearchByGroup(const std::string&, std::string&);
    bool GroupSearch(const std::string&, std::string&);
    bool GroupSearchById(const std::string&, std::string&);
    bool InventorSearch(const std::string&, std::string&);
    bool InfluenceSearchByGroup(const string& input, string& output);

    //weibo services
    bool WeiboSearch(const std::string&, std::string&);
    bool UserSearch(const std::string&, std::string&);
    bool WeiboSearchByUser(const string& input, string& output);
    bool InfluenceSearchByUser(const string& input, string& output);
    bool UserSearchById(const string& input, string& output);

private:
    std::unique_ptr<AMinerData> aminer;
    std::unique_ptr<PMinerData> pminer;
    std::unique_ptr<WeiboData> weibo;
};
