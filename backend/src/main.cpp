#include "rpc/RpcServer.hpp"
#include "search.hpp"
#include "aminerdata.hpp"

using namespace std;

void setup_services(RpcServer* server) {
    server->addMethod("PubSearch", &demoserver::PubSearch);
    server->addMethod("AuthorSearch", &demoserver::AuthorSearch);
    server->addMethod("AuthorPubSearch", &demoserver::AuthorPublicationSearch);
}

int main() {
    AMinerData& aminer = AMinerData::instance();
    RpcServer* server = RpcServer::CreateServer(40112, 20);
    setup_services(server);
    server->run();
    return 0;
}
