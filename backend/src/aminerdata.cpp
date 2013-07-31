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
            string text = p.title + " " + p.abstract;
            pub_index.addSingle(ai->GlobalId(), 0, text, avgLen);
        }
    }
    cerr << "index built!" << endl;
}
