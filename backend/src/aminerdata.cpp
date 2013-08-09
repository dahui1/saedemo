#include <atomic>
#include <algorithm>
#include <ctype.h>
#include "glog/logging.h"
#include "aminerdata.hpp"
#include "thread_util.hpp"
#include "indexing/analyzer.hpp"

using namespace std;
using namespace sae::io;
using namespace indexing;

AMinerData::AMinerData(char const * prefix) {
    LOG(INFO) << "loading aminer graph...";
    g.reset(MappedGraph::Open(prefix));
    int publication_type = g->VertexTypeIdOf("Publication");
    LOG(INFO) << "calculating average length...";
    auto calc_avg_len = [&]() {
        double avgLen = 0;
        int count = 0;
        for (auto ai = g->Vertices(); ai->Alive(); ai->Next()) {
            if (ai->TypeId() == publication_type) {
                count++;
                auto p = parse<Publication>(ai->Data());
                for (unsigned i = 0; i < p.title.length(); i++)
                    if (p.title[i] == ' ' && i != p.title.length()-1)
                        avgLen++;
                avgLen++;
                for (unsigned i = 0; i < p.abstract.length(); i++)
                    if (p.abstract[i] == ' ' && i != p.abstract.length()-1)
                        avgLen++;
                avgLen++;
            }
        }
        LOG(INFO) << "count: " << count << ", avgLen: " << avgLen;
        return avgLen / count;
    };
    double avgLen = 150; //calc_avg_len();

    LOG(INFO) << "building index...";
    const int shards = thread::hardware_concurrency();
    pub_index_shards.resize(shards);

    auto offset = g->VerticesOfType("Publication")->GlobalId();
    auto total = g->VertexCountOfType("Publication");
    auto shard_size = total / shards;
    atomic<int> processed(0);
    
    LOG(INFO) << "mapping author id to author name...";
    auto authoroff = g->VerticesOfType("Author")->GlobalId();
    auto authortotal = authoroff + g->VertexCountOfType("Author");
    auto author = g->Vertices();
    for (auto i = authoroff; i < authortotal && author->Alive(); i++, author->MoveTo(i)) {
        if (author->TypeName() == "Author") {
            auto a = parse<Author>(author->Data());
            int id = author->GlobalId();
            if (a.names.size() == 0) continue;
            string name = a.names[0];
            int hindex = a.h_index;
            id2name[id] = make_pair (name, hindex);
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            auto f = name2id.find(name);
            if (f == name2id.end()) {
                vector<pair<int,int>> names;
                names.push_back(make_pair (id, hindex));
                name2id[name] = names;
            }
            else {
                f->second.push_back(make_pair(id, hindex));
            }
        }
    }
    auto index_builder = [&](int shard_id) {
        auto ai = g->Vertices();
        auto start = offset + shard_id * shard_size;
        auto end = offset + (shard_id + 1) * shard_size;
        LOG(INFO) << "shard " << shard_id << " processing range: " << start << " to " << end;
        for (auto i = start; i < end && ai->Alive(); ai->MoveTo(i)) {
            if (ai->TypeId() == publication_type){
                auto p = parse<Publication>(ai->Data());
                string text = p.title + " " + p.abstract;
                auto ap = ai->InEdges();
                while (ap->Alive()) {
                    if (ap->TypeName() == "Publish") {
                        auto temp = id2name.find(ap->TargetId());
                        if (temp != id2name.end())
                        text += " " + temp->second.first;
                    }
                    ap->Next();
                }
                unique_ptr<TokenStream> stream(ArnetAnalyzer::tokenStream(text));
                pub_index_shards[shard_id].addSingle(ai->GlobalId(), 0, stream, avgLen);
            }
            // counter
            i++;
            if ((i - start) % 10000 == 0) {
                processed += 10000;
                LOG(INFO) << "[" << shard_id << "] progress: " << processed.load() << "/" << total << "\t" << (double)(processed.load()) / total;
            }
        }
        LOG(INFO) << "[" << shard_id << "] finished!";
    };
    dispatch_thread_group(index_builder, shards);

    LOG(INFO) << "index built!";
}

AMinerData::~AMinerData() {
    LOG(INFO) << "releasing aminer data...";
}

SearchResult AMinerData::search(const string& type, const string& query, int limit) const {
    if (type == "Publication" || type == "JConf") {
        return search_publications(query, limit);
    }
    LOG(ERROR) << "Search type not supported!";
    return SearchResult();
}

SearchResult AMinerData::search_publications(const string& query, int limit) const {
    vector<SearchResult> results(pub_index_shards.size());
    auto index_searcher = [&](int shard_id) {
        Searcher basic_searcher(pub_index_shards[shard_id]);
        unique_ptr<TokenStream> stream(ArnetAnalyzer::tokenStream(query));
        auto result = basic_searcher.search(stream);
        std::sort(result.begin(), result.end());
        results[shard_id] = move(result);
    };
    dispatch_thread_group(index_searcher, results.size());

    // merge the results.
    // TODO: improve this merge algorithm.
    for (int i = results.size() - 1; i > 0; i--) {
        int dst = i / 2;
        auto dsize = results[dst].size();
        results[dst].insert(results[dst].end(), results[i].begin(), results[i].end());
        std::inplace_merge(results[dst].begin(), results[dst].begin() + dsize, results[dst].end());
        results[i].clear();
        if (results[dst].size() > limit)
            results[dst].resize(limit);
    }

    return results[0];
}
