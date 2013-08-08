#include <algorithm>
#include <vector>
#include <unordered_map>
#include <string>

#include "glog/logging.h"
#include "indexing/indexing.hpp"

template<class DatasetType>
struct SupportDocSearcher {

	SupportDocSearcher(DatasetType& data, const std::string& support_doc_type, int support_doc_limit)
		: data(data),
		  support_doc_type(support_doc_type),
		  support_doc_limit(support_doc_limit) {
	}

	virtual ~SupportDocSearcher() {
	}

	indexing::SearchResult search(std::string query, int limit = 5000) {
		indexing::SearchResult result;
		auto g = data.g.get();
		std::unordered_map<int, std::vector<indexing::QueryItem>> target_supports;

		DLOG(INFO) << "Searching for query: " << query;
		auto enumer = data.search(support_doc_type, query, support_doc_limit);

		DLOG(INFO) << "Aggregating support docs, size = " << enumer.size();
		for (auto& support : enumer) {
			auto targets = this->get_targets(support);
			for (auto& target : targets) {
				target_supports[target].push_back(support);
			}
		}

		DLOG(INFO) << "Scoring targets, size = " << target_supports.size();
		for (auto& ts : target_supports) {
			indexing::QueryItem item{ts.first, this->get_score(ts)};
			if (item.score > 20 || result.size() < limit) {
				result.push_back(item);
			}
		}

		DLOG(INFO) << "Sorting...";
		std::sort(result.begin(), result.end());

		if (result.size() > limit)
			result.resize(limit);

		return result;
	}

protected:
	virtual std::vector<sae::io::vid_t> get_targets(indexing::QueryItem support) {
		throw "get_targets not implemented";
	}

	virtual double get_score(const std::pair<int, std::vector<indexing::QueryItem>>& target_supports) {
		double score = 0;
		for (auto& support : target_supports.second) {
			score += support.score;
		}
		return score;
	}

	DatasetType& data;
	std::string support_doc_type;
	int support_doc_limit;
};
