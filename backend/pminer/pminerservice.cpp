#include "interface.pb.h"
#include "pminerdata.hpp"
#include "entitysearcher.hpp"
#include "indexing/indexing.hpp"
#include "zrpc/zrpc.hpp"
#include "glog/logging.h"
#include "searchservice.hpp"
#include "event.hpp"

using namespace std;
using namespace std::chrono;
using namespace demoserver;
using namespace indexing;
using namespace sae::io;
using namespace zrpc;


DEFINE_string(pminer, "pminer", "prefix of pminer graph");


struct PMinerService : public SearchServiceBase {

    PMinerService(unique_ptr<IndexedGraph>&& ig) : SearchServiceBase(std::move(ig)) {
    }

    SERVICE(PatentSearch, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto query = request.query();
        auto results = searcher.search(WeightedType{{"Patent", 1.0}}, WeightedType{{"Patent", 1.0}}, query);
        fillSearchResponse(request, response, results);
        return true;
    }

    SERVICE(GroupSearch, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"Group", 1.0}}, WeightedType{{"Patent", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

    SERVICE(InventorSearch, EntitySearchRequest, EntitySearchResponse) {
        auto searcher = EntitySearcher(ig.get());
        auto results = searcher.search(WeightedType{{"Inventor", 1.0}}, WeightedType{{"Patent", 1.0}}, request.query());
        fillSearchResponse(request, response, results);
        return true;
    }

protected:

    void fillEntity(DetailedEntity* de, VertexIterator* vi) {
        if (vi->TypeName() == "Patent") {
            auto pat = parse<Patent>(vi->Data());
            de->set_id(vi->GlobalId());
            de->set_title(pat.title);
            de->set_url(to_string(pat.pn));
        } else if (vi->TypeName() == "Group") {
            auto group = parse<Group>(vi->Data());
            de->set_id(vi->GlobalId());
            de->set_title(group.name);
            de->set_imgurl(group.imgurl);
        } else if (vi->TypeName() == "Inventor") {
            auto inventor = parse<Inventor>(vi->Data());
            de->set_id(vi->GlobalId());
            de->set_title(inventor.name);
        }
    }
};

#define ADD_METHOD(name) server->addMethod(#name, b(&PMinerService::name))

static void init(void *sender, void *args) {
    RpcServer *server = reinterpret_cast<RpcServer *>(args);
    LOG(INFO) << "loading pminer graph: " << FLAGS_pminer;
    unique_ptr<PMinerData> ig(new PMinerData(FLAGS_pminer.c_str()));
    PMinerService *service = new PMinerService(std::move(ig));
    auto b = zrpc::make_binder(*service);
    ADD_METHOD(PatentSearch);
    ADD_METHOD(GroupSearch);
    ADD_METHOD(InventorSearch);
    LOG(INFO) << "pminer initialized. ";
}

REGISTER_EVENT(init_service, init);
