#include <string>
#include "rpc/RpcServer.hpp"

struct AnalyzeService {
    AnalyzeService() {}
    virtual ~AnalyzeService() {}

    static AnalyzeService* CreateService();
    virtual void attachTo(sae::rpc::RpcServer* server) = 0;
};
