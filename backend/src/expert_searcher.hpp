#pragma once
#include "aminerdata.hpp"
#include "support_doc_searcher.hpp"
#include <algorithm>

struct ExpertSearcher : public SupportDocSearcher<AMinerData> {
    ExpertSearcher(AMinerData& aminer) : SupportDocSearcher(aminer, "Publication", 5000) {
    }
    ~ExpertSearcher() {
    }

    indexing::SearchResult search(std::string query, int limit = 5000) {
        auto g = data.g.get();
        indexing::SearchResult result;
        unordered_map<int, std::vector<indexing::QueryItem>> author_pubs;
        LOG(INFO) << "Searching for query: " << query;
        LOG(INFO) << "Checking if the query is a name..";
        //check if the query is a name
        string lowerquery = query;
        std::transform(lowerquery.begin(), lowerquery.end(), lowerquery.begin(), ::tolower);
        cout << lowerquery << endl;
        auto name = data.name2id.find(lowerquery);
        if (name != data.name2id.end()) {
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
                result.push_back(indexing::QueryItem{id, 1000000});
                auto a = g->Vertices();
                a->MoveTo(id);
                for (auto pubs = a->OutEdges(); pubs->Alive(); pubs->Next()) {
                    if (pubs->TypeName() == "Publish") {
                        auto pid = pubs->TargetId();
                        auto pub = data.get<Publication>(pid);
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
                    indexing::QueryItem item{iter->first, (double)iter->second};
                    result.push_back(item);
                }
                LOG(INFO) << "Sorting...";
                std::sort(result.begin(), result.end());

                if (result.size() > 5000)
                    result.resize(5000);

                return result;
            }
        }
        return SupportDocSearcher::search(query);
    }

protected:
    virtual std::vector<sae::io::vid_t> get_targets(indexing::QueryItem support) {
        std::vector<sae::io::vid_t> targets;

        auto& g = data.g;
        auto vi = g->Vertices();
        vi->MoveTo(support.docId);
        auto publish_type = g->EdgeTypeIdOf("Publish");
        for (auto edgeIt = vi->InEdges(); edgeIt->Alive(); edgeIt->Next()) {
            if (edgeIt->TypeId() == publish_type) {
                int aid = edgeIt->SourceId();
                targets.push_back(aid);
            }
        }

        return targets;
    }
    
    virtual double get_score(const std::pair<int, std::vector<indexing::QueryItem>>& author_pubs) {
        auto author = data.get<Author>(author_pubs.first);
        double score = 0.0;

        for (const indexing::QueryItem& item : author_pubs.second)
        {
            auto pub = data.get<Publication>(item.docId);
            int nCitations = pub.citation_number;
            if (nCitations >= 1)
                score += item.score * sqrt((double)nCitations) / 2.0;
            else
                score += item.score;
        }

        return score;
    }
};
