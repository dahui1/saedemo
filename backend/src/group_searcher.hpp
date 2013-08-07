#pragma once
#include "indexing/search.hpp"
#include "pminerdata.hpp"

class GroupSearcher
{
public:
    GroupSearcher(const PMinerData& pminer, int max_pat_count = 5000);
    ~GroupSearcher();
    indexing::SearchResult search(std::string query);
private:
    const PMinerData& pminer;
    int pat_count;
};

class InventorSearcher
{
public:
    InventorSearcher(const PMinerData& pminer, int max_pat_count = 5000);
    ~InventorSearcher();
    indexing::SearchResult search(std::string query);
private:
    const PMinerData& pminer;
    int pat_count;
};
