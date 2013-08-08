#include <unordered_map>
#include <cstring>
#include <algorithm>
#include <ctype.h>
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
    LOG(INFO) << "Searching for query: " << query;
    LOG(INFO) << "Checking if the query is a name..";    
    //check if the query is a name
    string lowerquery = query;
    std::transform(lowerquery.begin(), lowerquery.end(), lowerquery.begin(), ::tolower);
    cout << lowerquery << endl;
    auto name = aminer.name2id.find(lowerquery);
    if (name != aminer.name2id.end()) {
        auto list = name->second;
        int hindex = 0;
        int id = -1;
        for (pair<int, int> author : list) {
            if (author.second > hindex) {
                hindex = author.second;
                id = author.first;
            }
        }
        if (id != -1) {
            unordered_map<int, int> authorlist;
            result.push_back(QueryItem{id, 1000000});
            auto a = g->Vertices();
            a->MoveTo(id);
            for (auto pubs = a->OutEdges(); pubs->Alive(); pubs->Next()) {
                if (pubs->TypeName() == "Publish") {
                    auto pid = pubs->TargetId();
                    auto pub = aminer.get<Publication>(pid);
                    int citation = pub.citation_number;
                    for (auto auts = pubs->Target()->InEdges(); auts->Alive(); auts->Next()) {
                        int aid = auts->SourceId();
                        if (aid == id) continue;
                        auto findaid = authorlist.find(aid);
                        if (findaid == authorlist.end()) 
                            authorlist[aid] = citation;
                        else
                            findaid->second += citation;
                    }
                }
            }
            for (auto iter = authorlist.begin(); iter != authorlist.end(); iter++) {
                QueryItem item{iter->first, (double)iter->second};
                result.push_back(item);
            }
            LOG(INFO) << "Sorting...";
            std::sort(result.begin(), result.end());

            if (result.size() > 5000)
                result.resize(5000);

            return result;
        }
    }

    // searching section
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

