#include "pminer.hpp"
#include "pminerdata.hpp"
#include "io/mgraph.hpp"

using namespace sae::io;
using namespace std;

int main() 
{
    PMinerData pminer("pminer");
    auto g = pminer.g.get();
    auto ver = g->Vertices();
    auto v = g->Edges();
    for (; v->Alive(); v->Next()) {
        if (v->TypeName() == "PatentCompany") {
            ver->MoveTo(v->TargetId());
            auto group = parse<Company>(ver->Data());
            ver->MoveTo(v->SourceId());
            auto patent = parse<Patent>(ver->Data());
            cout << "Company: " << group.name << "; Title: " << patent.title << endl;
        }
    }
    cerr << "finished.." << endl;
}
