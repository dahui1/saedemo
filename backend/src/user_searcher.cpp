#include <unordered_map>
#include <cstring>
#include <algorithm>
#include "glog/logging.h"
#include "user_searcher.hpp"
#include "indexing/indexing.hpp"

using namespace std;
using namespace indexing;
using namespace sae::io;

double get_score(const pair<int, vector<QueryItem>>& user_wbs, const WeiboData& weibo)
{
    auto user = weibo.get<User>(user_wbs.first);
    double score = 0.0;
    for (const QueryItem& item : user_wbs.second)
    {	
        auto wb = weibo.get<Weibo>(item.docId);
        score += item.score * (wb.reposts_count + wb.comments_count);
    }
    return score;
}

UserSearcher::UserSearcher(const WeiboData& weibo, int max_wb_count)
    :weibo(weibo), wb_count(max_wb_count)
{
}

UserSearcher::~UserSearcher()
{
}

SearchResult UserSearcher::search(string query)
{
    auto g = weibo.g.get();
    SearchResult result;
    unordered_map<int, vector<QueryItem>> user_wbs;

    //searching section
    LOG(INFO) << "Searching for query: " << query;
    auto enumer = weibo.search_weibos(query, wb_count);

    LOG(INFO) << "Generating User Weibos... weibos: " << enumer.size();
    auto weibo_type = g->EdgeTypeIdOf("Weibo");
    for (auto& item : enumer) {
        auto vi = g->Vertices();
        vi->MoveTo(item.docId);
        for (auto edgeIt = vi->InEdges(); edgeIt->Alive(); edgeIt->Next()) {
            if (edgeIt->TypeName() == "UserWeibo") {
                int gid = edgeIt->SourceId();
                user_wbs[gid].push_back(item);
            }
        }
    }

    LOG(INFO) << "Scoring... users: " << user_wbs.size();
    //start caculation score
    for (auto& uwbs : user_wbs) {
        QueryItem item{uwbs.first, get_score(uwbs, weibo)};
        if (item.score > 20 || result.size() <= 5000)
            result.push_back(item);
    }
    
    LOG(INFO) << "Sorting...";
    std::sort(result.begin(), result.end());
    if (result.size() > 5000)
        result.resize(5000);
    
    return result;
}

