#pragma once
#include <vector>
#include "indexing/query.hpp"
#include "io/mgraph.hpp"

struct ExtractFeatureParam
{
    int doc_id;
};

struct SupportDocFeatureParam: public ExtractFeatureParam
{
    std::vector<indexing::QueryItem> support_docs;
};

double conf_score(int conf_id, sae::io::MappedGraph* g);
