#pragma once
#include "indexing/search.hpp"
#include "indexing/query.hpp"
#include "features.hpp"
#include "aminer.hpp"
#include "io/mgraph.hpp"

/*
* We only provide document based expert search( search expert by search corresponding publications first)
* This class provide interface for feature plugin, thus one could add possible features and their weight to this class.
* each feature extractor could enumerate the candidate and return a score, a overall sum of feature score would be
* used for ranking the expert
*/

class ExpertSearcher
{
public:
	ExpertSearcher(indexing::Index& index, int max_pub_count = 5000);
	~ExpertSearcher();
	indexing::SearchResult search(std::string query, sae::io::MappedGraph* g);
private:
	indexing::Index& index;
	int pub_count;
};

double get_score(ExtractFeatureParam* param_ptr, sae::io::MappedGraph* g);
