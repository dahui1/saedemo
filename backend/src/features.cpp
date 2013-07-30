#include "features.hpp"
#include "indexing.hpp"
#include "aminer.hpp"
#include <math.h>

// XXX what's this?
#define SEARCH_STEP 100

using namespace sae::io;

namespace indexing{

double conf_score(int conf_id, const std::unique_ptr<MappedGraph>& g)
	// XXX fix this!
{//if conf_id == -1 return sqrt(pub->nCitations);
//	auto vc = g->Vertices();
//	vc->MoveTo(conf_id);
//	JConf jc = sae::serialization::convert_from_string<JConf>(vc->Data());
	return 1;//jc.id;
}
}

