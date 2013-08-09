#pragma once
#include "indexing/search.hpp"
#include "pminerdata.hpp"

struct GroupSearcher : public SupportDocSearcher<PMinerData> {
public:
    GroupSearcher(PMinerData& pminer) : SupportDocSearcher(pminer, "Patent", 5000) {
    }
    ~GroupSearcher() {
    }

protected:
    virtual std::vector<sae::io::vid_t> get_targets(indexing::QueryItem support) {
        std::vector<sae::io::vid_t> targets;

        auto vi = data.g->Vertices();
        vi->MoveTo(support.docId);
        for (auto eit = vi->OutEdges(); eit->Alive(); eit->Next()) {
            if (eit->TypeName() == "PatentGroup") {
                int gid = eit->TargetId();
                targets.push_back(gid);
            }
        }

        return targets;
    }

    virtual double get_score(const std::pair<int, std::vector<indexing::QueryItem>>& gpats) {
        auto group = data.get<Group>(gpats.first);
        return SupportDocSearcher::get_score(gpats);
    }
};

struct InventorSearcher : public SupportDocSearcher<PMinerData>
{
    InventorSearcher(PMinerData& pminer) : SupportDocSearcher(pminer, "Patent", 5000) {
    }
    ~InventorSearcher() {
    }

protected:
    virtual std::vector<sae::io::vid_t> get_targets(indexing::QueryItem support) {
        std::vector<sae::io::vid_t> targets;

        auto vi = data.g->Vertices();
        vi->MoveTo(support.docId);
        for (auto eit = vi->InEdges(); eit->Alive(); eit->Next()) {
            if (eit->TypeName() == "PatentInventor") {
                int gid = eit->SourceId();
                targets.push_back(gid);
            }
        }

        return targets;
    }
};
