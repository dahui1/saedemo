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

namespace demoserver {

    bool sortByScore(const QueryItem& A, const QueryItem& B) {
        return A.score > B.score;
    }

    bool AuthorSearch(const string& input, string& output) {
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
        AMinerData& aminer = AMinerData::instance();
        auto result = ExpertSearcher(aminer.getPubIndex()).search(query, aminer.getGraph());
        EntitySearchResponse response;
        response.set_total_count(count);
        response.set_query(query);
        for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
            auto i = result[ri];
            DetailedEntity *de = response.add_entity();
            auto p = aminer.get<Author>(i.docId);
            de->set_title(p.names[0]);
            de->set_id(i.docId);
            de->set_description(p.position + p.affiliation);
            de->set_imgurl(p.imgurl);
        }
        return response.SerializeToString(&output);
    }

    bool AuthorPublicationSearch(const string& input, string& output) {
        EntitySearchRequest request;
        request.ParseFromString(input);
        string query = request.query();
        AMinerData& aminer = AMinerData::instance();

        auto result = ExpertSearcher(aminer.getPubIndex()).search(query, aminer.getGraph());
        auto vi = aminer.getGraph()->Vertices();
        map<vid_t, bool> publications;
        EntitySearchResponse response;
        response.set_query(query);
        int count = 0;
        for (auto& i : result) {
            vi->MoveTo(i.docId);
            auto edgeIt = vi->OutEdges();
            while (edgeIt->Alive()) {
                if (edgeIt->Typename() == "Publish") {
                    auto pit = publications.find(i.docId);
                    if (pit != publications.end()) {
                        edgeIt->Next();
                        continue;
                    }
                    publications[i.docId] = true;
                    DetailedEntity *de = response.add_entity();
                    auto vt = edgeIt->Target();
                    auto p = sae::serialization::convert_from_string<Publication>(vt->Data());
                    de->set_title(p.title);
                    de->set_id(i.docId);
                    de->set_description(p.abstract);
                }
                edgeIt->Next();
            }
        }
        response.set_total_count(publications.size());
        return response.SerializeToString(&output);
    }

    bool PubSearch(const string& input, string& output) {
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

        AMinerData& aminer = AMinerData::instance();
        auto result = Searcher(aminer.getPubIndex()).search(query);

        if (result.size() > 5000)
            result.resize(5000);

        std::sort(result.begin(), result.end(), sortByScore);
        EntitySearchResponse response;
        response.set_total_count(result.size());
        response.set_query(query);
        for (int ri = offset; ri < result.size() && ri - offset < count; ri++) {
            auto i = result[ri];
            DetailedEntity *de = response.add_entity();
            auto p = aminer.get<Publication>(i.docId);
            de->set_title(p.title);
            de->set_id(i.docId);
            de->set_description(p.abstract);
        }
        return response.SerializeToString(&output);
    }
}
