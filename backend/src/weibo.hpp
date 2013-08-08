#pragma once
#include <string>
#include <vector>
#include "serialization/serialization_includes.hpp"

using std::vector;
using std::string;

struct User {
    string avatar_large;
    int bi_followers_count;
    string id;
    int followers_count;
    string profile_url;
    int statuses_count;
    string description;
    int friends_count;
    int mbrank;
    string profile_image_url;
    string name;
    string weihao;
    string remark;
    int favourites_count;
    string url;
    string gender;
    string created_at;
};

struct Weibo {
    string user_id;
    string created_at;
    string text;
    int reposts_count;
    int comments_count;
};

struct UserWeibo {
};

struct UserInfluence {
    int weight;
};

namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, User> {
                static void run(sae::serialization::OSerializeStream& ostr, User& u) {
                    ostr << u.avatar_large << u.bi_followers_count << u.id << u.followers_count
                         << u.profile_url << u.statuses_count << u.description << u.friends_count
                         << u.mbrank << u.profile_image_url << u.name << u.weihao << u.remark
                         << u.favourites_count << u.url << u.gender << u.created_at;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, User> {
                static void run(sae::serialization::ISerializeStream& istr, User& u) {
                    istr >> u.avatar_large >> u.bi_followers_count >> u.id >> u.followers_count
                         >> u.profile_url >> u.statuses_count >> u.description >> u.friends_count
                         >> u.mbrank >> u.profile_image_url >> u.name >> u.weihao >> u.remark
                         >> u.favourites_count >> u.url >> u.gender >> u.created_at;
                }
            };
        }
    }
}

namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, Weibo> {
                static void run(sae::serialization::OSerializeStream& ostr, Weibo& w) {
                    ostr << w.user_id << w.created_at << w.text << w.reposts_count << w.comments_count;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, Weibo> {
                static void run(sae::serialization::ISerializeStream& istr, Weibo& w) {
                    istr >> w.user_id >> w.created_at >> w.text >> w.reposts_count >> w.comments_count;
                }
            };
        }
    }
}

