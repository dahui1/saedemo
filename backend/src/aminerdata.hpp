#include <iostream>
#include <unordered_map>
#include <string>
#include "io/mgraph.hpp"
#include "serialization/serialization_includes.hpp"
#include "aminer.hpp"
#include "indexing/search.hpp"
#include "expert_searcher.hpp"
#include "indexing/analyzer.hpp"

template <typename T>
inline T parse(const string& data) {
    return sae::serialization::convert_from_string<T>(data);
}

template <typename T>
inline std::unique_ptr<T> pparse(const string& data) {
    T *ret;
    std::stringstream stream;
    stream.str(data);
    sae::serialization::ISerializeStream decoder(&stream);
    decoder >> (*ret);
    return std::unique_ptr<T>(ret);
}

struct AMinerData {
    AMinerData(char const * prefix);

    ~AMinerData() {
    }

    indexing::Index& getAuthorIndex() {
        return author_index;
    }

    indexing::Index& getPubindex() {
        return pub_index;
    }

    indexing::Index& getJConfIndex() {
        return jconf_index;
    }

    sae::io::MappedGraph* getGraph() {
        return g.get();
    }

    static AMinerData& instance()
    {
        static AMinerData instance("aminer");
        return instance;
    }

    template<typename T>
    T get(sae::io::vid_t id) {
        auto vi = g->Vertices();
        vi->MoveTo(id);
        return parse<T>(vi->Data());
    }

    indexing::Index author_index, pub_index, jconf_index;
    std::unique_ptr<sae::io::MappedGraph> g;
};

