#pragma once
#include "pminer.hpp"
#include "indexedgraph.hpp"

namespace {
    std::map<std::string, std::string> patentDocExtractor(sae::io::VertexIterator* it) {
        Patent p = sae::serialization::convert_from_string<Patent>(it->Data());
        return std::map<std::string, std::string>{{"title", p.title}};
    }
}

struct PMinerData : public IndexedGraph {
    PMinerData(char const * prefix) : IndexedGraph(prefix) {
        buildIndexFor("Patent", patentDocExtractor);
    }
};
