#include <string>
#include "rpc/RpcServer.hpp"
#include "aminerdata.hpp"
#include "pminerdata.hpp"
#include "weibodata.hpp"
#include "pubmeddata.hpp"

struct SearchService {
	SearchService() {}
	virtual ~SearchService() {}

    static SearchService* CreateService(std::unique_ptr<AMinerData>&& adata, std::unique_ptr<PMinerData>&& pdata, std::unique_ptr<WeiboData>&& wdata, std::unique_ptr<PubmedData>&& meddata);
	virtual void attachTo(sae::rpc::RpcServer* server) = 0;
};
