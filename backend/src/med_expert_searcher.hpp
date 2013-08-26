#pragma once
#include "pubmeddata.hpp"
#include <algorithm>

struct MedExpertSearcher : public SupportDocSearcher<PubmedData> {
    MedExpertSearcher(PubmedData& pubmed) : SupportDocSearcher(pubmed, "Publication", 5000) {
    }
    ~MedExpertSearcher() {
    }

    indexing::SearchResult search(std::string query, int limit = 5000) {
        auto g = data.g.get();
        indexing::SearchResult result;
        LOG(INFO) << "Searching for query: " << query;
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
};

struct JournalSearcher : public SupportDocSearcher<PubmedData> {
    JournalSearcher(PubmedData& pubmed) : SupportDocSearcher(pubmed, "Journal", 5000) {
    }
    ~JournalSearcher() {
    }
    
protected:
    virtual std::vector<sae::io::vid_t> get_targets(indexing::QueryItem support) {
        std::vector<sae::io::vid_t> targets;

        auto vi = data.g->Vertices();
        vi->MoveTo(support.docId);
        for (auto eit = vi->OutEdges(); eit->Alive(); eit->Next()) {
            if (eit->TypeName() == "Appear") {
                int gid = eit->TargetId();
                targets.push_back(gid);
            }
        }
        return targets;
    }
};
