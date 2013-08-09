#include <chrono>
#include <algorithm>
#include <sstream>

#include "indexing/search.hpp"
#include "search.hpp"
#include "interface.pb.h"
#include "expert_searcher.hpp"
#include "group_searcher.hpp"
#include "user_searcher.hpp"

using namespace std;
using namespace demoserver;
using namespace indexing;
using namespace std::chrono;
using namespace sae::io;

struct SearchServiceImpl : public SearchService {
    SearchServiceImpl(std::unique_ptr<AMinerData>&& adata, std::unique_ptr<PMinerData>&& pdata, std::unique_ptr<WeiboData>&& wdata);
    ~SearchServiceImpl() {}

    //aminer services
    bool PubSearch(const std::string&, std::string&);
    bool PubSearchByAuthor(const std::string&, std::string&);
    bool AuthorSearch(const std::string&, std::string&);
    bool AuthorSearchById(const string& input, string& output);
    bool InfluenceSearchByAuthor(const string& input, string& output);
    bool JConfSearch(const string& input, string& output);

    //pminer services
    bool PatentSearch(const std::string&, std::string&);
    bool PatentSearchByInventor(const std::string&, std::string&);
    bool PatentSearchByGroup(const std::string&, std::string&);
    bool GroupSearch(const std::string&, std::string&);
    bool GroupSearchById(const std::string&, std::string&);
    bool InventorSearch(const std::string&, std::string&);
    bool InfluenceSearchByGroup(const string& input, string& output);

    //weibo services
    bool WeiboSearch(const std::string&, std::string&);
    bool UserSearch(const std::string&, std::string&);
    bool WeiboSearchByUser(const string& input, string& output);
    bool InfluenceSearchByUser(const string& input, string& output);
    bool UserSearchById(const string& input, string& output);

    void attachTo(sae::rpc::RpcServer* server) {
        LOG(INFO) << "Binding  services...";
        auto b = sae::rpc::make_binder(*this);

        //aminer services
        server->addMethod("PubSearch", b(&SearchServiceImpl::PubSearch));
        server->addMethod("PubSearchByAuthor", b(&SearchServiceImpl::PubSearchByAuthor));
        server->addMethod("AuthorSearch", b(&SearchServiceImpl::AuthorSearch));
        server->addMethod("AuthorSearchById", b(&SearchServiceImpl::AuthorSearchById));
        server->addMethod("InfluenceSearchByAuthor", b(&SearchServiceImpl::InfluenceSearchByAuthor));
        server->addMethod("JConfSearch", b(&SearchServiceImpl::JConfSearch));

        //pminer services
        server->addMethod("PatentSearch", b(&SearchServiceImpl::PatentSearch));
        server->addMethod("PatentSearchByGroup", b(&SearchServiceImpl::PatentSearchByGroup));
        server->addMethod("PatentSearchByInventor", b(&SearchServiceImpl::PatentSearchByInventor));
        server->addMethod("GroupSearch", b(&SearchServiceImpl::GroupSearch));
        server->addMethod("GroupSearchById", b(&SearchServiceImpl::GroupSearchById));
        server->addMethod("InventorSearch", b(&SearchServiceImpl::InventorSearch));
        server->addMethod("InfluenceSearchByGroup", b(&SearchServiceImpl::InfluenceSearchByGroup));

        //weibo services
        server->addMethod("UserSearch", b(&SearchServiceImpl::UserSearch));
        server->addMethod("UserSearchById", b(&SearchServiceImpl::UserSearchById));
        server->addMethod("WeiboSearch", b(&SearchServiceImpl::WeiboSearch));
        server->addMethod("WeiboSearchByUser", b(&SearchServiceImpl::WeiboSearchByUser));
        server->addMethod("InfluenceSearchByUser", b(&SearchServiceImpl::InfluenceSearchByUser));

        LOG(INFO) << "Services have been set up.";
    };

private:
    std::unique_ptr<AMinerData> aminer;
    std::unique_ptr<PMinerData> pminer;
    std::unique_ptr<WeiboData> weibo;
};

namespace {
    string join(const string& sep, const vector<string>& values) {
        std::stringstream ss;
        for(size_t i = 0; i < values.size(); ++i)
        {
            if(i != 0)
                ss << sep;
            ss << values[i];
        }
        return ss.str();
    }

    void fill_entity_by_author(DetailedEntity* de, const Author& author) {
        de->set_title(author.names[0]);
        de->set_original_id(author.id);
        de->set_description(author.position + ", " + author.affiliation);
        de->set_imgurl(author.imgurl);
        de->set_topics(join(",", author.topics));
        auto stat = de->add_stat();
        stat->set_type("h-index");
        stat->set_value(author.h_index);
        stat = de->add_stat();
        stat->set_type("citations");
        stat->set_value(author.citation_number);
        stat = de->add_stat();
        stat->set_type("publications");
        stat->set_value(author.publication_number);
    }

    void fill_entity_by_publication(DetailedEntity* de, const Publication& pub) {
        de->set_original_id(pub.id);
        de->set_title(pub.title);
        de->set_description(pub.abstract);
        de->set_topics(join(",", pub.topics));
        auto stat = de->add_stat();
        stat->set_type("year");
        stat->set_value(pub.year);
        stat = de->add_stat();
        stat->set_type("jconf");
        stat->set_value(pub.jconf);
        stat = de->add_stat();
        stat->set_type("citation");
        stat->set_value(pub.citation_number);
    }
}

SearchServiceImpl::SearchServiceImpl(std::unique_ptr<AMinerData>&& adata, std::unique_ptr<PMinerData>&& pdata, std::unique_ptr<WeiboData>&& wdata)
    : aminer(std::move(adata)), pminer(std::move(pdata)), weibo(std::move(wdata)) {
}

bool SearchServiceImpl::AuthorSearchById(const string& input, string& output) {
    EntityDetailRequest request;
    request.ParseFromString(input);

    EntitySearchResponse response;
    auto vi = aminer->g->Vertices();
    string query = "";
    int count = 0;
    for (auto& aid : request.id()) {
        vi->MoveTo(aid);
        if (vi->TypeName() == "Author") {
            auto a = parse<Author>(vi->Data());
            DetailedEntity *de = response.add_entity();
            de->set_id(aid);
            fill_entity_by_author(de, a);
            query += to_string(aid) + "\t";
            count ++;
        }
    }
    response.set_query(query);
    response.set_total_count(count);
    return response.SerializeToString(&output);
}

bool SearchServiceImpl::AuthorSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();

    int offset = request.has_offset() ? request.offset() : 0;
    int count = request.has_count() ? request.count() : 50;
    auto result = ExpertSearcher(*aminer).search(query);
    EntitySearchResponse response;
    response.set_total_count(result.size());
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = aminer->get<Author>(i.docId);
        de->set_id(i.docId);
        fill_entity_by_author(de, p);
    }
    return response.SerializeToString(&output);
}

bool SearchServiceImpl::JConfSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();
    int offset = request.has_offset() ? request.offset() : 0;
    int count = request.has_count() ? request.count() : 50;

    auto result = JConfSearcher(*aminer).search(query);
    EntitySearchResponse response;
    response.set_total_count(result.size());
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = aminer->get<JConf>(i.docId);
        de->set_id(i.docId);
        de->set_title(p.name);
    }
    return response.SerializeToString(&output);
}

// TODO merge into PubSearch
bool SearchServiceImpl::PubSearchByAuthor(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);

    EntitySearchResponse response;
    response.set_query(request.query());

    auto aid = stoi(request.query());
    auto vit = aminer->g->Vertices();
    vit->MoveTo(aid);
    int count = 0;
    for (auto eit = vit->OutEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "Publish") {
            auto vi = aminer->g->Vertices();
            vi->MoveTo(eit->TargetId());
            auto pub = parse<Publication>(vi->Data());
            DetailedEntity *de = response.add_entity();
            de->set_id(eit->TargetId());
            fill_entity_by_publication(de, pub);

            auto re = de->add_related_entity();
            re->set_type("Author");
            for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
                if (ei->TypeName() == "Publish") {
                    re->add_id(ei->SourceId());
                }
            }

            count ++;
        }
    }

    response.set_total_count(count);

    return response.SerializeToString(&output);
}

bool SearchServiceImpl::PubSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();

    int offset = request.has_offset() ? request.offset() : 0;
    int count = request.has_count() ? request.count() : 50;

    auto result = aminer->search_publications(query);

    if (result.size() > 5000)
        result.resize(5000);

    EntitySearchResponse response;
    response.set_total_count(result.size());
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto vi = aminer->g->Vertices();
        vi->MoveTo(i.docId);
        auto p = parse<Publication>(vi->Data());
        de->set_id(i.docId);
        fill_entity_by_publication(de, p);

        auto re = de->add_related_entity();
        re->set_type("Author");
        for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
            if (ei->TypeName() == "Publish") {
                re->add_id(ei->SourceId());
            }
        }
    }
    return response.SerializeToString(&output);
}

bool SearchServiceImpl::InfluenceSearchByAuthor(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);

    InfluenceSearchResponse response;

    auto aid = stoi(request.query());
    response.set_entity_id(aid);

    auto vit = aminer->g->Vertices();
    vit->MoveTo(aid);
    for (auto eit = vit->OutEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "Influence") {
            auto ai = parse<AuthorInfluence>(eit->Data());
            Influence *inf = response.add_influence();
            inf->set_id(eit->TargetId());
            inf->set_topic(ai.topic);
            inf->set_score(ai.score);
        }
    }
    for (auto eit = vit->InEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "Influence") {
            auto ai = parse<AuthorInfluence>(eit->Data());
            Influence *inf = response.add_influenced_by();
            inf->set_id(eit->SourceId());
            inf->set_topic(ai.topic);
            inf->set_score(ai.score);
        }
    }

    return response.SerializeToString(&output);
}

bool SearchServiceImpl::PatentSearchByGroup(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);

    EntitySearchResponse response;
    response.set_query(request.query());

    auto gid = stoi(request.query());
    auto vit = pminer->g->Vertices();
    vit->MoveTo(gid);
    int count = 0;
    for (auto eit = vit->InEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "PatentGroup") {
            auto vi = pminer->g->Vertices();
            vi->MoveTo(eit->SourceId());
            auto pat = parse<Patent>(vi->Data());
            DetailedEntity *de = response.add_entity();
            de->set_id(eit->TargetId());
            de->set_url(to_string(pat.pn));
            de->set_title(pat.title);
            auto stat = de->add_stat();
            stat->set_type("year");
            stat->set_value(pat.year);
            if (pat.inventors.size() > 0) {
                string inventors = pat.inventors[0];
                for (int i = 1; i < pat.inventors.size(); i++)
                    inventors += ", " + pat.inventors[i];
                de->set_description(inventors);
            }
            count ++;
        }
    }

    response.set_total_count(count);

    return response.SerializeToString(&output);
}

bool SearchServiceImpl::PatentSearchByInventor(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);

    EntitySearchResponse response;
    response.set_query(request.query());

    auto iid = stoi(request.query());
    auto vit = pminer->g->Vertices();
    vit->MoveTo(iid);
    int count = 0;
    for (auto eit = vit->OutEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "PatentInventor") {
            auto vi = pminer->g->Vertices();
            vi->MoveTo(eit->TargetId());
            auto pat = parse<Patent>(vi->Data());
            DetailedEntity *de = response.add_entity();
            de->set_id(eit->TargetId());
            de->set_url(to_string(pat.pn));
            de->set_title(pat.title);
            auto stat = de->add_stat();
            stat->set_type("year");
            stat->set_value(pat.year);

            auto re = de->add_related_entity();
            re->set_type("Inventor");
            for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
                if (ei->TypeName() == "PatentInventor") {
                    re->add_id(ei->SourceId());
                }
            }

            count ++;
        }
    }

    response.set_total_count(count);

    return response.SerializeToString(&output);
}

bool SearchServiceImpl::PatentSearch(const string& input, string& output) {

    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();

    int offset = request.has_offset() ? request.offset() : 0;
    int count = request.has_count() ? request.count() : 50;

    auto result = pminer->search_patents(query);

    if (result.size() > 5000)
        result.resize(5000);

    EntitySearchResponse response;
    response.set_total_count(result.size());
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = pminer->get<Patent>(i.docId);
        de->set_id(i.docId);
        de->set_url(to_string(p.pn));
        de->set_title(p.title);
        de->set_original_id(p.id);
        auto stat = de->add_stat();
        stat->set_type("year");
        stat->set_value(p.year);

        auto re = de->add_related_entity();
        re->set_type("Inventor");
        auto vi = pminer->g->Vertices();
        vi->MoveTo(i.docId);
        for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
            if (ei->TypeName() == "PatentInventor") {
                re->add_id(ei->SourceId());
            }
        }
    }
    return response.SerializeToString(&output);
}

bool SearchServiceImpl::GroupSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();

    int offset = request.has_offset() ? request.offset() : 0;
    int count = request.has_count() ? request.count() : 50;

    auto result = GroupSearcher(*pminer).search(query);
    EntitySearchResponse response;
    response.set_total_count(result.size());
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = pminer->get<Group>(i.docId);
        de->set_id(i.docId);
        de->set_title(p.name);
        de->set_imgurl(p.imgurl);
        auto stat = de->add_stat();
        stat->set_type("Patents");
        stat->set_value(p.patCount);
        de->set_original_id(p.id);
    }
    return response.SerializeToString(&output);
}

bool SearchServiceImpl::GroupSearchById(const string& input, string& output) {
    EntityDetailRequest request;
    request.ParseFromString(input);

    EntitySearchResponse response;
    string query = "";
    int count = 0;
    auto vi = pminer->g->Vertices();
    for (auto& gid : request.id()) {
        vi->MoveTo(gid);
        if (vi->TypeName() == "Group") {
            DetailedEntity *de = response.add_entity();
            auto p = parse<Group>(vi->Data());
            de->set_id(gid);
            de->set_title(p.name);
            de->set_imgurl(p.imgurl);
            auto stat = de->add_stat();
            stat->set_type("Patents");
            stat->set_value(p.patCount);
            de->set_original_id(p.id);
            query += to_string(gid) + "\t";
            count ++;
        }
    }
    response.set_query(query);
    response.set_total_count(count);
    return response.SerializeToString(&output);
}

bool SearchServiceImpl::InventorSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();
    int offset = request.has_offset() ? request.offset() : 0;
    int count = request.has_count() ? request.count() : 50;

    auto result = InventorSearcher(*pminer).search(query);
    EntitySearchResponse response;
    response.set_total_count(result.size());
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = pminer->get<Inventor>(i.docId);
        de->set_id(i.docId);
        de->set_title(p.name);
    }
    return response.SerializeToString(&output);
}

bool SearchServiceImpl::InfluenceSearchByGroup(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);

    InfluenceSearchResponse response;

    auto gid = stoi(request.query());
    response.set_entity_id(gid);

    auto vit = pminer->g->Vertices();
    vit->MoveTo(gid);
    for (auto eit = vit->OutEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "GroupInfluence") {
            auto gi = parse<GroupInfluence>(eit->Data());
            Influence *inf = response.add_influence();
            inf->set_id(eit->TargetId());
            inf->set_topic(gi.topic);
            inf->set_score(gi.score);
        }
    }
    for (auto eit = vit->InEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "GroupInfluence") {
            auto gi = parse<GroupInfluence>(eit->Data());
            Influence *inf = response.add_influenced_by();
            inf->set_id(eit->SourceId());
            inf->set_topic(gi.topic);
            inf->set_score(gi.score);
        }
    }

    return response.SerializeToString(&output);
}


bool SearchServiceImpl::WeiboSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();
    
    int offset = request.has_offset() ? request.offset() : 0;
    int count = request.has_count() ? request.count() : 50;
    auto result = weibo->search_weibos(query);

    if (result.size() > 5000)
        result.resize(5000);
    EntitySearchResponse response;
    response.set_total_count(result.size());
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = weibo->get<Weibo>(i.docId);
        de->set_title(p.text);
        de->set_id(i.docId);
        auto stat = de->add_stat();
        stat->set_type("Reposts");
        stat->set_value(p.reposts_count);
        stat = de->add_stat();
        stat->set_type("Comments");
        stat->set_value(p.comments_count);
        de->set_description(p.created_at);

        auto re = de->add_related_entity();
        re->set_type("Author");
        auto vi = weibo->g->Vertices();
        for (auto ei = vi->InEdges(); ei->Alive(); ei->Next()) {
            if (ei->TypeName() == "UserWeibo") {
                re->add_id(ei->SourceId());
            }
        }
    }
    return response.SerializeToString(&output);
}


bool SearchServiceImpl::UserSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();

    int offset = request.has_offset() ? request.offset() : 0;
    int count = request.has_count() ? request.count() : 50;

    auto result = UserSearcher(*weibo).search(query);
    EntitySearchResponse response;
    response.set_total_count(count);
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = weibo->get<User>(i.docId);
        de->set_id(i.docId);
        de->set_title(p.name);
        de->set_description(p.description);
        de->set_imgurl(p.profile_image_url);
        auto stat = de->add_stat();
        stat->set_type("Followers");
        stat->set_value(p.followers_count);
        // p.id is too long for int
        de->set_url(p.id);
    }
    return response.SerializeToString(&output);
}

bool SearchServiceImpl::InfluenceSearchByUser(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);

    InfluenceSearchResponse response;

    auto uid = stoi(request.query());
    response.set_entity_id(uid);

    auto vit = weibo->g->Vertices();
    vit->MoveTo(uid);
    for (auto eit = vit->InEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "UserInfluence") {
            auto ui = parse<UserInfluence>(eit->Data());
            Influence *inf = response.add_influence();
            inf->set_id(eit->TargetId());
            inf->set_topic(-1);
            inf->set_score(ui.weight);
        }
    }
    for (auto eit = vit->OutEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "UserInfluence") {
            auto ui = parse<UserInfluence>(eit->Data());
            Influence *inf = response.add_influenced_by();
            inf->set_id(eit->SourceId());
            inf->set_topic(-1);
            inf->set_score(ui.weight);
        }
    }

    return response.SerializeToString(&output);
}


bool SearchServiceImpl::WeiboSearchByUser(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);

    EntitySearchResponse response;
    response.set_query(request.query());

    auto uid = stoi(request.query());
    auto vit = weibo->g->Vertices();
    vit->MoveTo(uid);
    int count = 0;
    for (auto eit = vit->OutEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "UserWeibo") {
            auto vi = weibo->g->Vertices();
            vi->MoveTo(eit->TargetId());
            auto weibo = parse<Weibo>(vi->Data());
            DetailedEntity *de = response.add_entity();
            de->set_title(weibo.text);
            de->set_id(eit->TargetId());
            auto stat = de->add_stat();
            stat->set_type("Reposts");
            stat->set_value(weibo.reposts_count);
            stat = de->add_stat();
            stat->set_type("Comments");
            stat->set_value(weibo.comments_count);
            de->set_description(weibo.created_at);

            count ++;
        }
    }
    response.set_total_count(count);

    return response.SerializeToString(&output);
}

bool SearchServiceImpl::UserSearchById(const string& input, string& output) {
    EntityDetailRequest request;
    request.ParseFromString(input);

    EntitySearchResponse response;
    string query = "";
    response.set_query(query);

    int count = 0;
    auto vi = weibo->g->Vertices();
    for (auto& uid : request.id()) {
        vi->MoveTo(uid);
        if (vi->TypeName() == "User") {
            DetailedEntity *de = response.add_entity();
            auto p = parse<User>(vi->Data());
            de->set_id(uid);
            de->set_title(p.name);
            de->set_description(p.description);
            de->set_imgurl(p.profile_image_url);
            auto stat = de->add_stat();
            stat->set_type("Followers");
            stat->set_value(p.followers_count);
            de->set_url(p.id);
            count ++;
        }
    }
    response.set_total_count(count);
    return response.SerializeToString(&output);
}

SearchService* SearchService::CreateService(std::unique_ptr<AMinerData>&& adata, std::unique_ptr<PMinerData>&& pdata, std::unique_ptr<WeiboData>&& wdata) {
    return new SearchServiceImpl(std::move(adata), std::move(pdata), std::move(wdata));
}
