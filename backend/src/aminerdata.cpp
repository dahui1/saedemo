#include "aminerdata.hpp"

using namespace std;
using namespace sae::io;
using namespace indexing;

AMinerData::AMinerData(char const * prefix) {
    cerr << "loading aminer graph..." << endl;
    g.reset(MappedGraph::Open(prefix));
    cerr << "building index..." << endl;
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
    cout << "count: " << cout << endl;
    avgLen /= count;
    cout << "avgLen: " << avgLen << endl;
    for (auto ai = g->Vertices(); ai->Alive(); ai->Next()) {
        if (ai->GlobalId() % 100000 == 0)
            cerr << "Parsing vertex: " << ai->GlobalId() << ", type: " << ai->Typename() << endl;
        if (ai->Typename() == "Publication"){
            auto p = parse<Publication>(ai->Data());
            unique_ptr<TokenStream> stream(ArnetAnalyzer::tokenStream(p.title + " " + p.abstract));
            unordered_map<int, vector<short>> word_position;
            int position = 0;

            Token token;
            while (stream->next(token)) {
                string term = token.getTermText();
                int term_id = pub_index.word_map.id(term);
                word_position[term_id].push_back(position++);
            }
               int totalTokens = position;
            for (auto& wp : word_position) {
                int word_id = wp.first;
                auto& positions = wp.second;
                int freq = static_cast<int>(positions.size());
                double score = (freq * (BM25_K + 1)) / (freq + BM25_K * (1 - BM25_B + BM25_B * totalTokens / avgLen));
                // insert a new posting item, global id cannot be larger than max int
                pub_index[Term{word_id, 0}].insert(PostingItem{static_cast<int>(ai->GlobalId()), positions, score});
            }
        }
    }
    cerr << "index built!" << endl;
}
