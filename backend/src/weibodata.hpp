#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include "io/mgraph.hpp"
#include "serialization/serialization_includes.hpp"
#include "weibo.hpp"
#include "indexing/search.hpp"
#include "indexing/analyzer.hpp"
#include "aminerdata.hpp"

struct WeiboData {
    WeiboData(char const * prefix);

    ~WeiboData() {
    }

    template<typename T>
    T get(sae::io::vid_t id) const {
        auto vi = g->Vertices();
        vi->MoveTo(id);
        return parse<T>(vi->Data());
    }

    indexing::SearchResult search_weibos(const std::string& query, int limit = 5000) const;
    indexing::SearchResult search(const std::string& type, const string& query, int limit = 5000) const {
        if (type == "Weibo") {
            return search_weibos(query, limit);
        }
        return indexing::SearchResult();
    }

    std::vector<indexing::Index> weibo_index_shards;
    std::unique_ptr<sae::io::MappedGraph> g;
};

