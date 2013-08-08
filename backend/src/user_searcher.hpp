#pragma once
#include "indexing/search.hpp"
#include "weibodata.hpp"

class UserSearcher
{
public:
    UserSearcher(const WeiboData& weibo, int max_wb_count = 5000);
    ~UserSearcher();
    indexing::SearchResult search(std::string query);
private:
    const WeiboData& weibo;
    int wb_count;
};
