#pragma once
#include <string>
#include <vector>
#include "serialization/serialization_includes.hpp"

using std::vector;
using std::string;

struct User {
    string domain;
    string avatar_large;
    int bi_followers_count;
    int block_word;
    int star;
    string id;
    int city;
    bool verified;
    //bool follow_me;
    //string verified_reason;
    int followers_count;
    string location;
    int mbtype;
    string profile_url;
    int province;
    int statuses_count;
    string description;
    int friends_count;
    //int online_status;
    int mbrank;
    //string idstr;
    string profile_image_url;
    //bool allow_all_act_msg;
    //bool allow_all_comment;
    //bool geo_enabled;
    string name;
    string lang;
    string weihao;
    string remark;
    int favourites_count;
    string screen_name;
    string url;
    string gender;
    string created_at;
    //string verified_type;
    //bool following;
};

struct     Weibo {
    string user_id;
    string created_at;
    string text;
    int reposts_count;
    int comments_count;
};

struct UserWeibo {
    
};


namespace sae {
    namespace serialization {
        namespace custom_serialization_impl {
            template <>
            struct serialize_impl<sae::serialization::OSerializeStream, User> {
                static void run(sae::serialization::OSerializeStream& ostr, User& u) {
                    ostr << u.domain << u.avatar_large << u.bi_followers_count << u.block_word << u.star
                            << u.id << u.city << u.verified << u.followers_count << u.location << u.mbtype
                            << u.profile_url << u.province << u.statuses_count << u.description << u.friends_count
                            << u.mbrank << u.profile_image_url << u.name << u.lang << u.weihao << u.remark
                            << u.favourites_count << u.screen_name << u.url << u.gender << u.created_at;
                }
            };

            template <>
            struct deserialize_impl<sae::serialization::ISerializeStream, User> {
                static void run(sae::serialization::ISerializeStream& istr, User& u) {
                    istr >> u.domain >> u.avatar_large >> u.bi_followers_count >> u.block_word >> u.star
                            >> u.id >> u.city >> u.verified >> u.followers_count >> u.location >> u.mbtype
                            >> u.profile_url >> u.province >> u.statuses_count >> u.description >> u.friends_count
                            >> u.mbrank >> u.profile_image_url >> u.name >> u.lang >> u.weihao >> u.remark
                            >> u.favourites_count >> u.screen_name >> u.url >> u.gender >> u.created_at;
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

