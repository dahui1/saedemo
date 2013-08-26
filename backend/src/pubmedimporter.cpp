#include <iostream>
#include <fstream>
#include <unordered_map>
#include "csvreader.hpp"
#include "io/mgraph.hpp"
#include "pubmed.hpp"

using namespace std;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

#define AUTHOR_BASE 0
#define PUB_BASE (1LL << 32)
#define JOURNAL_BASE (1LL << 33)

int main() {
    sae::io::vid_t gvid = 0, geid = 0;

    // GraphBuilder is not efficient for this kind of large data.
    cerr << "graph builder..." << endl;
    sae::io::GraphBuilder<uint64_t> builder;
    builder.AddVertexDataType("Authors");
    builder.AddVertexDataType("Pub");
    builder.AddVertexDataType("Journal");
    builder.AddEdgeDataType("Publish");
    builder.AddEdgeDataType("Appear");
    
    unordered_map<string, int> author2id;
    unordered_map<string, int> journal2id;

    // build Authors
    cerr << "building pubs, authors and journals..." << endl;
    {
        ifstream all_csv("output.csv");
        CSVReader all_reader(all_csv);
        vector<string> names;
        int p = 0, a = 0, j = 0;
        string journal;
        Pub pub;
        vector<string> row;
        all_reader.readrow(row); // header
        while (all_reader.readrow(row)) {
            pub.title = row[0];
            pub.publisher = row[1];
            pub.year = stoi(row[4]);
            pub.abstract = row[5];
            pub.keywords = split(row[6], ',');
            
            journal = row[2];
            names = split(row[3], ',');
            
            builder.AddVertex(PUB_BASE + p, pub, "Pub");
            for (string name : names) {
                auto f = author2id.find(name);
                if (f == author2id.end()) {
                    builder.AddVertex(AUTHOR_BASE + a, Authors{name}, "Authors");
                    author2id[name] = a;
                    a++;
                }
                f = author2id.find(name);
                builder.AddEdge(AUTHOR_BASE + f->second, PUB_BASE + p, AuthorPub(), "Publish");
            }
            auto jo = journal2id.find(journal);
            if (jo == journal2id.end()) {
                builder.AddVertex(JOURNAL_BASE + j, Journal{journal}, "Journal");
                journal2id[journal] = j;
                j++;
            }
            jo = journal2id.find(journal);
            builder.AddEdge(PUB_BASE + p, JOURNAL_BASE + jo->second, PubJournal(), "Appear");
        }
    }

    cerr << "saving graph..." << endl;
    // save graph
    builder.Save("pubmed");
    cerr << "saved graph..." << endl;
    return 0;
}
