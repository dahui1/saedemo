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

ExpertSearcher::ExpertSearcher(Index& _index, int max_pub_count)
    :index(_index),
    pub_count(max_pub_count)
{
}

ExpertSearcher::~ExpertSearcher()
{
}

SearchResult ExpertSearcher::search(string query, MappedGraph* g)
{
    SearchResult result;
    SearchResult enumer;
    Publication pub;
    unordered_map<int, vector<QueryItem>> author_pubs;

    //Init each featrues for ranking
    vector<string> query_words;
    string::size_type start = 0, index;
    string substring;
    do {
        index = query.find_first_of(" ", start);
        if (index != string::npos) {
            substring = query.substr(start, index - start);
            query_words.push_back(substring);
            start = query.find_first_not_of(" ", index);
            if (start == string::npos)
                break;
        }
    } while (index != string::npos);
    substring = query.substr(start);
    query_words.push_back(substring);

    //searching section
    Searcher basic_searcher(this->index);
    enumer = basic_searcher.search(query);

    QueryItem item;
    vector<QueryItem>::iterator it = enumer.begin();
    for(int i = 0; i < enumer.size(); i++)
    {
        vector<vid_t> authorIDs;
        auto vi = g->Vertices();
        vi->MoveTo(enumer[i].docId);
        pub = sae::serialization::convert_from_string<Publication>(vi->Data());
        if(&pub)
        {
            auto edgeIt = vi->InEdges();
            while (edgeIt->Alive()) {
                if (edgeIt->Typename() == "Publish") {
                    authorIDs.push_back(edgeIt->SourceId());
                }
                edgeIt->Next();
            }
            vector<vid_t>::iterator ait = authorIDs.begin();
            for (vid_t naid : authorIDs )
            {
                if(naid!=-1)
                    author_pubs[naid].push_back(enumer[i]);
            }
        }
    }
    //start caculation score using each feature
    for(auto author_pubs_iter = author_pubs.begin(); author_pubs_iter!= author_pubs.end(); ++author_pubs_iter)
    {
        /*if(special_ids.find(author_pubs_iter->first)!=special_ids.end())
            item.score = 50000;
        else
            item.score = 0.0;*/
        item.score = 0.0;
        item.docId = author_pubs_iter->first;

        SupportDocFeatureParam param;
        param.doc_id = item.docId;
        param.support_docs = author_pubs_iter->second;
        item.score += get_score(&param, g);
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

