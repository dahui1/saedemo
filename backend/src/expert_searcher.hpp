#pragma once
#include "aminerdata.hpp"
#include "support_doc_searcher.hpp"

struct ExpertSearcher : public SupportDocSearcher<AMinerData> {
    ExpertSearcher(AMinerData& aminer) : SupportDocSearcher(aminer, "Publication", 5000) {
    }
    ~ExpertSearcher() {
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
