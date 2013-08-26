#include <algorithm>
#include <sstream>

#include "analyze.hpp"
#include "interface.pb.h"
#include "HIS/HIS.hpp"
#include "networks/networks.hpp"
#include "glog/logging.h"

using namespace std;
using namespace demoserver;
using namespace sae::io;

struct AnalyzeServiceImpl : public AnalyzeService {
    AnalyzeServiceImpl();
    ~AnalyzeServiceImpl() {}
    
    bool Analyze(const std::string&, std::string&);

    void attachTo(sae::rpc::RpcServer* server) {
        LOG(INFO) << "Binding analyze services...";
        auto b = sae::rpc::make_binder(*this);

        server->addMethod("Analyze", b(&AnalyzeServiceImpl::Analyze));
        LOG(INFO) << "Services have been set up.";
    };
};

AnalyzeServiceImpl::AnalyzeServiceImpl()
{
}

AnalyzeService* AnalyzeService::CreateService() {
    return new AnalyzeServiceImpl();
}

bool AnalyzeServiceImpl::Analyze(const std::string& input, std::string& output) {
    AnalyzeRequest request;
    request.ParseFromString(input);
    
    AnalyzeResponse response;
    string path;
    vector<his::Outdata> us;
    response.set_algorithm(request.algorithm());
    response.set_path("");
    path = request.path();
    if (request.algorithm() == "HIS") {
        us = runHIS(path,"HIS");
    }
    else if(request.algorithm() == "Pagerank")
    {
	us = runHIS(path,"Pagerank");
    }
    else {
        us = runNetworks(path);
    }
    for (auto usrscore : us) {
        auto temp = response.add_us();
        temp->set_id(usrscore.id);
        temp->set_score(usrscore.score);
        temp->set_name(usrscore.name);
        temp->set_pagerank(usrscore.pagerank);
        for (auto c : usrscore.communities)
            temp->add_communities(c);
        for (auto n : usrscore.neighbor) {
            auto com = temp->add_neighbor();
            com->set_id(n.id);
            com->set_name(n.name);
            com->set_score(n.score);
            for (auto c : n.communities)
                com->add_communities(c);
        }
        for (auto e : usrscore.nei_edges) {
            auto nei = temp->add_edge();
            nei->set_f(e.f);
            nei->set_l(e.l);
        }
    }

    return response.SerializeToString(&output);
}
