#include <unordered_map>
#include <cstring>
#include <algorithm>
#include "glog/logging.h"
#include "expert_searcher.hpp"
#include "indexing/indexing.hpp"

using namespace std;
using namespace indexing;
using namespace sae::io;

namespace {

    double get_score(const pair<int, vector<QueryItem>>& author_pubs, const AMinerData& aminer)
    {
        auto author = aminer.get<Author>(author_pubs.first);
        double score = 0.0;

        for (const QueryItem& item : author_pubs.second)
        {
            auto pub = aminer.get<Publication>(item.docId);
            int confID = pub.jconf;
            int nCitations = pub.citation_number;
            if(/*confID==-1 &&*/ nCitations>=1)
                score += item.score * sqrt((double)nCitations)/2.0;
            else
                score += item.score;
        }

        double h_index = author.h_index;
        //score += h_index;
        return score;
    }

}


ExpertSearcher::ExpertSearcher(const AMinerData& aminer, int max_pub_count)
    : aminer(aminer), pub_count(max_pub_count)
{
}

ExpertSearcher::~ExpertSearcher()
{
}

SearchResult ExpertSearcher::search(string query)
{
    auto g = aminer.g.get();
    SearchResult result;
    unordered_map<int, vector<QueryItem>> author_pubs;

    // searching section
    LOG(INFO) << "Searching for query: " << query;
    auto enumer = aminer.search_publications(query, pub_count);

    LOG(INFO) << "Generating Author Pubs... publications: " << enumer.size();
    auto publish_type = g->EdgeTypeIdOf("Publish");
    for (auto& item : enumer) {
        auto vi = g->Vertices();
        vi->MoveTo(item.docId);
        for (auto edgeIt = vi->InEdges(); edgeIt->Alive(); edgeIt->Next()) {
            if (edgeIt->TypeId() == publish_type) {
                int aid = edgeIt->SourceId();
                author_pubs[aid].push_back(item);
            }
        }
    }

    LOG(INFO) << "Scoring... authors:  " << author_pubs.size();
    // start caculation scores
    for(auto& apubs : author_pubs)
    {
        QueryItem item{apubs.first, get_score(apubs, aminer)};
        if (item.score > 30 || result.size() <= 5000)
             result.push_back(item);
    }

    LOG(INFO) << "Sorting...";
    std::sort(result.begin(), result.end());

    if (result.size() > 5000)
        result.resize(5000);

    return result;
}

