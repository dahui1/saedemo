#include <unordered_map>
#include <cstring>
#include <algorithm>
#include "expert_searcher.hpp"
#include "indexing/indexing.hpp"

using namespace std;
using namespace indexing;
using namespace sae::io;

bool sortByScore(const QueryItem& A, const QueryItem& B) {
    return A.score > B.score;
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
    auto g = aminer.getGraph();
    SearchResult result;
    unordered_map<int, vector<QueryItem>> author_pubs;

    // searching section
    auto enumer = aminer.search_publications(query);

    for (auto& item : enumer) {
        vector<vid_t> authorIDs;
        auto vi = g->Vertices();
        vi->MoveTo(item.docId);
        auto pub = sae::serialization::convert_from_string<Publication>(vi->Data());
        for (auto edgeIt = vi->InEdges(); edgeIt->Alive(); edgeIt->Next()) {
            if (edgeIt->Typename() == "Publish") {
                authorIDs.push_back(edgeIt->SourceId());
            }
        }
        for (auto naid : authorIDs )
        {
            author_pubs[naid].push_back(item);
        }
    }

    // start caculation score using each feature
    for(auto& apubs : author_pubs)
    {
        SupportDocFeatureParam param;
        param.doc_id = apubs.first;
        param.support_docs = apubs.second;

        QueryItem item{apubs.first, get_score(&param, g)};
        if (item.score > 30 || result.size() <= 5000)
             result.push_back(item);
    }

    std::sort(result.begin(), result.end(), sortByScore);
    if (result.size() > 5000)
        result.resize(5000);

    return result;
}

double get_score(ExtractFeatureParam* param_ptr, MappedGraph* g)
{
    auto vi = g->Vertices();
    vi->MoveTo(param_ptr->doc_id);
    auto author = sae::serialization::convert_from_string<Author>(vi->Data());
    Publication pub;
    double score = 0.0;
    if(&author)
    {
        for (QueryItem& item : ((SupportDocFeatureParam*)param_ptr)->support_docs)
        {
            auto vp = g->Vertices();
            vp->MoveTo(item.docId);
            pub = sae::serialization::convert_from_string<Publication>(vp->Data());
            if(&pub)
            {
                int confID = pub.jconf;
                int nCitations = pub.citation_number;
                if(/*confID==-1 &&*/ nCitations>=1)
                    score += item.score * sqrt((double)nCitations)/2.0;
                else
                    score += item.score;
            }
        }
        // XXX what's this?
        double h_index = author.h_index;
        //score+=h_index;
    }
    return score;
}

