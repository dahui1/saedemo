#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include "io/mgraph.hpp"
#include "serialization/serialization_includes.hpp"
#include "pubmed.hpp"
#include "indexing/search.hpp"
#include "indexing/analyzer.hpp"
#include "aminerdata.hpp"

struct PubmedData {
    PubmedData(char const * prefix);
    ~PubmedData();

    template<typename T>
    T get(sae::io::vid_t id) const {
        auto vi = g->Vertices();
        vi->MoveTo(id);
        return parse<T>(vi->Data());
    }

    indexing::SearchResult search(const std::string& type, const std::string& query, int limit = 5000) const;
    indexing::SearchResult search_publications(const std::string& query, int limit = 5000) const;

    std::vector<indexing::Index> pub_index_shards;
    std::unique_ptr<sae::io::MappedGraph> g;
};

