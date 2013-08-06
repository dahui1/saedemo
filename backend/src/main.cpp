#include <memory>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "rpc/RpcServer.hpp"
#include "search.hpp"
#include "aminerdata.hpp"

DEFINE_int32(port, 40112, "server port");
DEFINE_int32(threads, 20, "server threads");
DEFINE_string(aminer, "aminer", "graph prefix of aminer");

using namespace std;
using namespace sae::rpc;

void setup_services(RpcServer* server) {
    LOG(INFO) << "Loading aminer data...";
    auto aminer = unique_ptr<AMinerData>(new AMinerData(FLAGS_aminer.c_str()));

    LOG(INFO) << "Making aminer services...";
    // Note that this object have to be allocated on heap
    auto service = new SearchService(std::move(aminer));

    LOG(INFO) << "Binding aminer services...";
    auto b = make_binder(*service);
    server->addMethod("PubSearch", b(&SearchService::PubSearch));
    server->addMethod("PubSearchByAuthor", b(&SearchService::PubSearchByAuthor));
    server->addMethod("AuthorSearch", b(&SearchService::AuthorSearch));
    server->addMethod("AuthorSearchById", b(&SearchService::AuthorSearchById));
    server->addMethod("InfluenceSearchByAuthor", b(&SearchService::InfluenceSearchByAuthor));

    LOG(INFO) << "AMiner services have been set up.";
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
