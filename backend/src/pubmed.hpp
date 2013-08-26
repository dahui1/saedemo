#pragma once
#include <string>
#include <vector>
#include "serialization/serialization_includes.hpp"

using std::vector;
using std::string;

struct Authors {
    string name;
};

struct Pub {
    int year;
    string title;
    string abstract;
    string publisher;
    vector<string> keywords;
};

struct Journal {
    string name;
};

struct AuthorPub {
    // the position-th author in the author list
    int position;
};

struct PubJournal {
    // Noop
};

namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, Authors> {
                static void run(sae::serialization::OSerializeStream& ostr, Authors& a) {
                    ostr << a.name;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, Authors> {
                static void run(sae::serialization::ISerializeStream& istr, Authors& a) {
                    istr >> a.name;
                }
            };
        }
    }
}

namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, Pub> {
                static void run(sae::serialization::OSerializeStream& ostr, Pub& p) {
                    ostr << p.year << p.title << p.abstract << p.publisher << p.keywords;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, Pub> {
                static void run(sae::serialization::ISerializeStream& istr, Pub& p) {
                    istr >> p.year >> p.title >> p.abstract >> p.publisher >> p.keywords;
                }
            };
        }
    }
}

namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, Journal> {
                static void run(sae::serialization::OSerializeStream& ostr, Journal& j) {
                    ostr << j.name;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, Journal> {
                static void run(sae::serialization::ISerializeStream& istr, Journal& j) {
                    istr >> j.name;
                }
            };
        }
    }
}

