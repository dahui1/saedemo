#include <memory>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "rpc/RpcServer.hpp"
#include "search.hpp"
#include "aminerdata.hpp"
#include "pminerdata.hpp"
#include "weibodata.hpp"

DEFINE_int32(port, 40112, "server port");
DEFINE_int32(threads, 20, "server threads");
DEFINE_string(aminer, "aminer", "graph prefix of aminer");
DEFINE_string(pminer, "pminer", "graph prefix of pminer");
DEFINE_string(weibo, "weibo", "graph prefix of weibo");

using namespace std;
using namespace sae::rpc;

void setup_services(RpcServer* server) {
    LOG(INFO) << "Loading aminer data...";
    auto aminer = unique_ptr<AMinerData>(new AMinerData(FLAGS_aminer.c_str()));
    LOG(INFO) << "Loading pminer data...";
    auto pminer = unique_ptr<PMinerData>(new PMinerData(FLAGS_pminer.c_str()));
    LOG(INFO) << "Loading weibo data...";
    auto weibo = unique_ptr<WeiboData>(new WeiboData(FLAGS_weibo.c_str()));

    LOG(INFO) << "Making aminer, pminer and weibo services...";
    // Note that this object have to be allocated on heap
    auto service = new SearchService(std::move(aminer), std::move(pminer), std::move(weibo));

    LOG(INFO) << "Binding aminer and pminer services...";
    auto b = make_binder(*service);

    //aminer services
    server->addMethod("PubSearch", b(&SearchService::PubSearch));
    server->addMethod("PubSearchByAuthor", b(&SearchService::PubSearchByAuthor));
    server->addMethod("AuthorSearch", b(&SearchService::AuthorSearch));
    server->addMethod("AuthorSearchById", b(&SearchService::AuthorSearchById));
    server->addMethod("InfluenceSearchByAuthor", b(&SearchService::InfluenceSearchByAuthor));

    //pminer services
    server->addMethod("PatentSearch", b(&SearchService::PatentSearch));
    server->addMethod("PatentSearchByGroup", b(&SearchService::PatentSearchByGroup));
    server->addMethod("PatentSearchByInventor", b(&SearchService::PatentSearchByInventor));
    server->addMethod("GroupSearch", b(&SearchService::GroupSearch));
    server->addMethod("InventorSearch", b(&SearchService::InventorSearch));
    server->addMethod("InfluenceSearchByGroup", b(&SearchService::InfluenceSearchByGroup));

    //weibo services
    server->addMethod("UserSearch", b(&SearchService::UserSearch));
    server->addMethod("WeiboSearch", b(&SearchService::WeiboSearch));

    LOG(INFO) << "Services have been set up.";
}


int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    LOG(INFO) << "Demo Server Starting...";
    RpcServer* server = RpcServer::CreateServer(FLAGS_port, FLAGS_threads);

    LOG(INFO) << "Setting up services...";
    setup_services(server);

    LOG(INFO) << "Trying to bringing our services up...";
    server->run();

    LOG(INFO) << "Exiting...";
    return 0;
}
