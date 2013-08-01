#include "gflags/gflags.h"
#include "rpc/RpcServer.hpp"
#include "search.hpp"
#include "aminerdata.hpp"

DEFINE_int32(port, 40112, "server port");
DEFINE_int32(threads, 20, "server threads");
DEFINE_string(aminer, "aminer", "graph prefix of aminer");

using namespace std;
using namespace sae::rpc;

void setup_services(RpcServer* server) {
    AMinerData aminer(FLAGS_aminer.c_str());
    SearchService service(aminer);
    auto b = make_binder(service);
    server->addMethod("PubSearch", b(&SearchService::PubSearch));
    server->addMethod("AuthorSearch", b(&SearchService::AuthorSearch));
    server->addMethod("AuthorPubSearch", b(&SearchService::AuthorPublicationSearch));
}

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    RpcServer* server = RpcServer::CreateServer(FLAGS_port, FLAGS_threads);
    setup_services(server);
    server->run();
    return 0;
}
