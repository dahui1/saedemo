#pragma once
#include "io/mgraph.hpp"
#include "indexing/search.hpp"
#include "features.hpp"
#include "aminerdata.hpp"

/*
* We only provide document based expert search( search expert by search corresponding publications first)
* This class provide interface for feature plugin, thus one could add possible features and their weight to this class.
* each feature extractor could enumerate the candidate and return a score, a overall sum of feature score would be
* used for ranking the expert
*/

class ExpertSearcher
{
public:
	ExpertSearcher(const AMinerData& aminer, int max_pub_count = 5000);
	~ExpertSearcher();
	indexing::SearchResult search(std::string query);
private:
	const AMinerData& aminer;
	int pub_count;
};

double get_score(ExtractFeatureParam* param_ptr, sae::io::MappedGraph* g);
