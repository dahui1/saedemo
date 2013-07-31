#include <atomic>
#include "aminerdata.hpp"
#include "thread_util.hpp"

using namespace std;
using namespace sae::io;
using namespace indexing;

AMinerData::AMinerData(char const * prefix) {
    cerr << "loading aminer graph..." << endl;
    g.reset(MappedGraph::Open(prefix));
    cerr << "calculating average length..." << endl;
    double avgLen = 0, count = 0;
    for (auto ai = g->Vertices(); ai->Alive(); ai->Next()) {
        if (ai->Typename() == "Publication") {
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
    cerr << "count: " << count << endl;
    avgLen /= count;
    cerr << "avgLen: " << avgLen << endl;

    cerr << "building index..." << endl;
    const int shards = thread::hardware_concurrency();
    pub_index_shards.resize(shards);

    auto total = g->VertexCount();
    auto shard_size = total / shards;
    decltype(shard_size) offset = 0;
    atomic<int> processed(0);
    auto index_builder = [&](int shard_id) {
        auto ai = g->Vertices();
        auto start = offset + shard_id * shard_size;
        auto end = offset + (shard_id + 1) * shard_size;
        cerr << "shard " << shard_id << " processing range: " << start << " to " << end << endl;
        for (auto i = start; i < end && ai->Alive(); ai->MoveTo(i)) {
            if (ai->Typename() == "Publication"){
                auto p = parse<Publication>(ai->Data());
                string text = p.title + " " + p.abstract;
                pub_index_shards[shard_id].addSingle(ai->GlobalId(), 0, text, avgLen);
            }
            i++;
            if ((i - start) % 10000 == 0) {
                processed += 10000;
                cerr << "[" << shard_id << "] progress: " << processed.load() << "/" << total << "\t" << (double)(processed.load()) / total << endl;
            }
        }
        cerr << "[" << shard_id << "] finished!" << endl;
    };
    dispatch_thread_group(index_builder, shards);

    cerr << "index built!" << endl;
}

SearchResult AMinerData::search_publications(const string& query) const {
    vector<SearchResult> results(pub_index_shards.size());
    auto index_searcher = [&](int shard_id) {
        Searcher basic_searcher(pub_index_shards[shard_id]);
        results[shard_id] = basic_searcher.search(query);
    };
    dispatch_thread_group(index_searcher, results.size());

    SearchResult result;
    for (auto& r : results) {
        result.insert(result.end(), r.begin(), r.end());
    }
    return result;
}
