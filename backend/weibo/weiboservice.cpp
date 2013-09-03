#include "glog/logging.h"
#include "indexing/search.hpp"
#include "interface.pb.h"
#include "event.hpp"
#include "entitysearcher.hpp"
#include "searchservice.hpp"
#include "weibodata.hpp"

using namespace std;
using namespace demoserver;
using namespace indexing;
using namespace sae::io;
using namespace zrpc;

DEFINE_string(weibo, "weibo", "weibo data prefix");

struct WeiboService : public SearchServiceBase {
    WeiboService(std::unique_ptr<IndexedGraph>&& ig) : SearchServiceBase(std::move(ig)) {
    }

    SERVICE(UserSearch, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"User", 1.0}}, WeightedType{{"Weibo", 1.0}, {"User", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

    SERVICE(WeiboSearch, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"Weibo", 1.0}}, WeightedType{{"Weibo", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

protected:
    void fillEntity(DetailedEntity* de, VertexIterator* vi) {
        de->set_id(vi->GlobalId());
        if (vi->TypeName() == "User") {
            auto p = parse<Weibo>(vi->Data());
            de->set_title(p.text);
            auto stat = de->add_stat();
            stat->set_type("Reposts");
            stat->set_value(p.reposts_count);
            stat = de->add_stat();
            stat->set_type("Comments");
            stat->set_value(p.comments_count);
            de->set_description(p.created_at);

            auto re = de->add_related_entity();
            re->set_type("Author");
            for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
                if (ei->TypeName() == "UserWeibo") {
                    re->add_id(ei->SourceId());
                }
            }
        } else if (vi->TypeName() == "Weibo") {
            auto p = parse<User>(vi->Data());
            de->set_title(p.name);
            de->set_description(p.description);
            de->set_imgurl(p.profile_image_url);
            auto stat = de->add_stat();
            stat->set_type("Followers");
            stat->set_value(p.followers_count);
            // p.id is too long for int
            de->set_url(p.id);
        }
    }
};

#define ADD_METHOD(name) server->addMethod(#name, b(&WeiboService::name))

static void init(void *sender, void *args) {
    RpcServer *server = reinterpret_cast<RpcServer *>(args);
    LOG(INFO) << "loading weibo graph: " << FLAGS_weibo;
    unique_ptr<WeiboData> ig(new WeiboData(FLAGS_weibo.c_str()));
    auto *service = new WeiboService(std::move(ig));
    auto b = zrpc::make_binder(*service);
    ADD_METHOD(UserSearch);
    ADD_METHOD(WeiboSearch);
    LOG(INFO) << "weibo initialized. ";
}

REGISTER_EVENT(init_service, init);
