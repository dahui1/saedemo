#pragma once
#include "indexing/search.hpp"
#include "weibodata.hpp"

class UserSearcher : public SupportDocSearcher<WeiboData>
{
public:
    UserSearcher(WeiboData& weibo) : SupportDocSearcher(weibo, "Weibo", 5000) {
    }
    ~UserSearcher() {
    }

protected:
    virtual std::vector<sae::io::vid_t> get_targets(indexing::QueryItem support) {
        std::vector<sae::io::vid_t> targets;

        auto& g = data.g;
        auto vi = g->Vertices();
        vi->MoveTo(support.docId);
        auto publish_type = g->EdgeTypeIdOf("UserWeibo");
        for (auto edgeIt = vi->InEdges(); edgeIt->Alive(); edgeIt->Next()) {
            if (edgeIt->TypeId() == publish_type) {
                int gid = edgeIt->SourceId();
                targets.push_back(gid);
            }
        }

        return targets;
    }

    virtual double get_score(const std::pair<int, std::vector<indexing::QueryItem>>& user_wbs)
    {
        double score = 0.0;

        for (const indexing::QueryItem& item : user_wbs.second)
        {
            auto wb = data.get<Weibo>(item.docId);
            score += item.score * (wb.reposts_count + wb.comments_count);
        }

        return score;
    }
};
