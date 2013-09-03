#pragma once
#include "weibo.hpp"
#include "indexedgraph.hpp"
#include "chinesetokenstream.hpp"

struct WeiboData : public IndexedGraph {
    WeiboData(char const * prefix) : IndexedGraph(prefix, ChineseTokenStream()) {
        buildIndexFor("Weibo", weiboDocExtractor);
        buildIndexFor("User", userDocExtractor);
    }

    static std::map<std::string, std::string> userDocExtractor(sae::io::VertexIterator* it) {
        auto d = sae::serialization::convert_from_string<User>(it->Data());
        return std::map<std::string, std::string>{{"name", d.name}};
    }

    static std::map<std::string, std::string> weiboDocExtractor(sae::io::VertexIterator* it) {
        auto d = sae::serialization::convert_from_string<Weibo>(it->Data());
        return std::map<std::string, std::string>{{"content", d.text}};
    }
};
