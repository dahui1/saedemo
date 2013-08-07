#ifndef OS_LINUX
#include <Windows.h>
#pragma comment(lib, "ICTCLAS50.lib")
#endif

#include <atomic>
#include <fstream>
#include <iostream>
#include <sstream>
#include "weibodata.hpp"
#include "glog/logging.h"
#include "thread_util.hpp"
#include "ICTCLAS50.h"

using namespace std;
using namespace sae::io;
using namespace indexing;

WeiboData::WeiboData(char const * prefix) {
    LOG(INFO) << "loading weibo graph...";
    g.reset(MappedGraph::Open(prefix));
    int weibo_type = g->VertexTypeIdOf("Weibo");
    LOG(INFO) << "calculating average length...";
    auto calc_avg_len = [&]() {
        double avgLen = 0;
        int count = 0;
        for (auto ai = g->Vertices(); ai->Alive(); ai->Next()) {
            if (ai->TypeId() == weibo_type) {
                count++;
                auto w = parse<Weibo>(ai->Data());
                for (unsigned i = 0; i < w.text.length(); i++)
                    if (w.text[i] == ' ' && i != w.text.length()-1)
                        avgLen++;
                avgLen++;
            }
        }
        LOG(INFO) << "count: " << count << ", avgLen: " << avgLen;
        return avgLen / count;
    };
	double avgLen = 22;
	
	LOG(INFO) << "loading stopword dic...";
	ifstream stopword_file("stopword.txt");
	std::set<string> stopwords;
	string stopword_input;
	while (getline(stopword_file, stopword_input)) {
		stopwords.insert(stopword_input);
	}
	stopword_file.close();

    LOG(INFO) << "building index...";
    const int shards = thread::hardware_concurrency();
    weibo_index_shards.resize(shards);

	auto offset = g->VerticesOfType("Weibo")->GlobalId();
	auto total = g->VertexCountOfType("Weibo");
	auto shard_size  =total / shards;
	ICTCLAS_Init();
        ICTCLAS_SetPOSmap(2);
	atomic<int> processed(0);
	auto index_builder = [&](int shard_id) {
        auto ai = g->Vertices();
        auto start = offset + shard_id * shard_size;
        auto end = offset + (shard_id + 1) * shard_size;
        LOG(INFO) << "shard " << shard_id << " processing range: " << start << " to " << end;
        for (auto i = start; i < end && ai->Alive(); ai->MoveTo(i)) {
            if (ai->TypeId() == weibo_type){
                auto w = parse<Weibo>(ai->Data());
                string text = w.text;
                const char* sentence = text.c_str();
                unsigned int nPaLen=strlen(sentence); 
                char* sRst=0; 
                sRst=(char *)malloc(nPaLen*6); 
                int nRstLen=0;
                nRstLen = ICTCLAS_ParagraphProcess(sentence,nPaLen,sRst,CODE_TYPE_UNKNOWN,0);
                const set<string> stw = stopwords;
                unique_ptr<TokenStream> stream(ArnetAnalyzer::tokenStream(sRst, stopwords));
                weibo_index_shards[shard_id].addSingle(ai->GlobalId(), 0, stream, avgLen);
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
    ICTCLAS_Exit();
    LOG(INFO) << "weibo index built!";
}

SearchResult WeiboData::search_weibos(const string& query, int limit) const {
    if (!ICTCLAS_Init()) {
        cout << "Fails!" << endl;
        SearchResult rs;
        return rs;
    }
    vector<SearchResult> results(weibo_index_shards.size());
    auto index_searcher = [&](int shard_id) {
        Searcher basic_searcher(weibo_index_shards[shard_id]);
        unique_ptr<TokenStream> stream(ArnetAnalyzer::tokenStream(query, 1));
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
	ICTCLAS_Exit();
    return results[0];
}
