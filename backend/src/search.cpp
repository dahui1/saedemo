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

SearchService::SearchService(std::unique_ptr<AMinerData>&& adata, std::unique_ptr<PMinerData>&& pdata, std::unique_ptr<WeiboData>&& wdata)
    : aminer(std::move(adata)), pminer(std::move(pdata)), weibo(std::move(wdata)) {
}

bool SearchService::AuthorSearchById(const string& input, string& output) {
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

bool SearchService::AuthorSearch(const string& input, string& output) {
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

// TODO merge into PubSearch
bool SearchService::PubSearchByAuthor(const string& input, string& output) {
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

bool SearchService::PubSearch(const string& input, string& output) {
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

bool SearchService::InfluenceSearchByAuthor(const string& input, string& output) {
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

bool SearchService::PatentSearchByGroup(const string& input, string& output) {
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
            de->set_title(pat.title);
            auto stat = de->add_stat();
            stat->set_type("year");
            stat->set_value(pat.year);
            if (pat.inventors.size() > 0) {
                string inventors = pat.inventors[0];
                cout << pat.inventors[0] << endl;
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



bool SearchService::PatentSearchByInventor(const string& input, string& output) {
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


bool SearchService::PatentSearch(const string& input, string& output) {
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

bool SearchService::GroupSearch(const string& input, string& output) {
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

bool SearchService::GroupSearchById(const string& input, string& output) {
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

bool SearchService::InventorSearch(const string& input, string& output) {
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

bool SearchService::InfluenceSearchByGroup(const string& input, string& output) {
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


bool SearchService::WeiboSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();
    int offset, count;
    if (request.has_offset())
        offset = request.offset();
    else
        offset = 0;
    if (request.has_count())
        count = request.count();
    else
        count = 50;
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
    }
    return response.SerializeToString(&output);
}


bool SearchService::UserSearch(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);
    string query = request.query();

    int offset, count;
    if (request.has_offset())
        offset = request.offset();
    else
        offset = 0;
    if (request.has_count())
        count = request.count();
    else
        count = 50;

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

bool SearchService::InfluenceSearchByUser(const string& input, string& output) {
    EntitySearchRequest request;
    request.ParseFromString(input);

    InfluenceSearchResponse response;

    auto uid = stoi(request.query());
    response.set_entity_id(uid);

    auto vit = weibo->g->Vertices();
    vit->MoveTo(uid);
    for (auto eit = vit->OutEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "UserInfluence") {
            auto ui = parse<UserInfluence>(eit->Data());
            Influence *inf = response.add_influence();
            inf->set_id(eit->TargetId());
            inf->set_score(ui.score);
        }
    }
    for (auto eit = vit->InEdges(); eit->Alive(); eit->Next()) {
        if (eit->TypeName() == "UserInfluence") {
            auto ui = parse<UserInfluence>(eit->Data());
            Influence *inf = response.add_influenced_by();
            inf->set_id(eit->SourceId());
            inf->set_score(ui.score);
        }
    }

    return response.SerializeToString(&output);
}