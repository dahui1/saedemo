#include <chrono>
#include <algorithm>

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

SearchService::SearchService(std::unique_ptr<AMinerData>&& data)
    : aminer(std::move(data)) {
}

bool SearchService::AuthorSearch(const string& input, string& output) {
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
        count = 500;
    auto result = ExpertSearcher(*aminer).search(query);
    EntitySearchResponse response;
    response.set_total_count(count);
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = aminer->get<Author>(i.docId);
        de->set_title(p.names[0]);
        de->set_id(i.docId);
        de->set_description(p.position + p.affiliation);
        de->set_imgurl(p.imgurl);
    }
    return response.SerializeToString(&output);
}

bool SearchService::PubSearch(const string& input, string& output) {
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
        count = 500;

    auto result = aminer->search_publications(query);

    if (result.size() > 5000)
        result.resize(5000);

    EntitySearchResponse response;
    response.set_total_count(result.size());
    response.set_query(query);
    for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
        auto i = result[ri];
        DetailedEntity *de = response.add_entity();
        auto p = aminer->get<Publication>(i.docId);
        de->set_title(p.title);
        de->set_id(i.docId);
        de->set_description(p.abstract);
    }
    return response.SerializeToString(&output);
}
