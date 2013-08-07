#include <unordered_map>
#include <cstring>
#include <algorithm>
#include "glog/logging.h"
#include "group_searcher.hpp"
#include "indexing/indexing.hpp"

using namespace std;
using namespace indexing;
using namespace sae::io;

double get_score(const pair<int, vector<QueryItem>>& group_pats, const PMinerData& pminer)
{
    auto group = pminer.get<Group>(group_pats.first);
    double score = 0.0;
    for (const QueryItem& item : group_pats.second)
    {
        score += item.score;
    }
    score += group.patCount / 40;

    return score;
}

GroupSearcher::GroupSearcher(const PMinerData& pminer, int max_pat_count)
    :pminer(pminer), pat_count(max_pat_count)
{
}

GroupSearcher::~GroupSearcher()
{
}

SearchResult GroupSearcher::search(string query)
{
    auto g = pminer.g.get();
    SearchResult result;
    unordered_map<int, vector<QueryItem>> group_pats;

    //searching section
    LOG(INFO) << "Searching for query: " << query;
    auto enumer = pminer.search_patents(query, pat_count);

    LOG(INFO) << "Generating Group Pats... patents: " << enumer.size();
    for (auto& item : enumer) {
        auto vi = g->Vertices();
        vi->MoveTo(item.docId);
        for (auto edgeIt = vi->OutEdges(); edgeIt->Alive(); edgeIt->Next()) {
            if (edgeIt->TypeName() == "PatentGroup" && edgeIt->TargetId() < 4000000) {
                int gid = edgeIt->TargetId();
                group_pats[gid].push_back(item);
            }
        }
    }

    LOG(INFO) << "Scoring... groups: " << group_pats.size();
    //start caculation score
    for (auto& gpats : group_pats) {
        QueryItem item{gpats.first, get_score(gpats, pminer)};
        if (item.score > 20 || result.size() <= 5000)
            result.push_back(item);
    }
    
    LOG(INFO) << "Sorting...";
    std::sort(result.begin(), result.end());
    if (result.size() > 5000)
        result.resize(5000);
    
    return result;
}




double get_score_inventor(const pair<int, vector<QueryItem>>& inventor_pats, const PMinerData& pminer)
{
    double score = 0.0;
    for (const QueryItem& item : inventor_pats.second)
    {
        score += item.score;
    }
    return score;
}

InventorSearcher::InventorSearcher(const PMinerData& pminer, int max_pat_count)
    :pminer(pminer), pat_count(max_pat_count)
{
}

InventorSearcher::~InventorSearcher()
{
}

SearchResult InventorSearcher::search(string query)
{
    auto g = pminer.g.get();
    SearchResult result;
    unordered_map<int, vector<QueryItem>> inventor_pats;

    //searching section
    LOG(INFO) << "Searching for query: " << query;
    auto enumer = pminer.search_patents(query, pat_count);

    LOG(INFO) << "Generating Inventor Pats... patents: " << enumer.size();
    for (auto& item : enumer) {
        auto vi = g->Vertices();
        vi->MoveTo(item.docId);
        for (auto edgeIt = vi->InEdges(); edgeIt->Alive(); edgeIt->Next()) {
            if (edgeIt->TypeName() == "PatentInventor") {
                int gid = edgeIt->SourceId();
                inventor_pats[gid].push_back(item);
            }
        }
    }

    LOG(INFO) << "Scoring... inventors: " << inventor_pats.size();
    //start caculation score
    for (auto& gpats : inventor_pats) {
        QueryItem item{gpats.first, get_score_inventor(gpats, pminer)};
        if (item.score > 20 || result.size() <= 5000)
            result.push_back(item);
    }

    LOG(INFO) << "Sorting...";
    std::sort(result.begin(), result.end());
    if (result.size() > 5000)
        result.resize(5000);

    return result;
}

