#include <chrono>
#include <algorithm>
#include <sstream>

#include "indexing/search.hpp"
#include "search.hpp"
#include "interface.pb.h"
#include "aminerdata.hpp"
#include "expert_searcher.hpp"

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

SearchService::SearchService(std::unique_ptr<AMinerData>&& data)
    : aminer(std::move(data)) {
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
            query += to_string(aid);
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
