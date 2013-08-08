#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cstdio>
#include "io/mgraph.hpp"
#include "weibo.hpp"

using namespace std;

#define USER_BASE 0
#define WEIBO_BASE 200000
//#define INVENTOR_BASE 4500000

vector<string> split(string s, char c) {
    int last = 0;
    vector<string> v;
    for (int i=0; i<s.size(); i++) {
        if (s[i] == c) {
            v.push_back(s.substr(last, i - last));
            last = i + 1;
        }
    }
    v.push_back(s.substr(last, s.size() - last));
    return v;
}

int convert_to_int(string s) {
    istringstream is;
    is.str(s);
    int i;
    is >> i;
    return i;
}

bool convert_to_bool(string s) {
    return s == "true" ? true : false;
}

int main() {
    sae::io::GraphBuilder<uint64_t> builder;
    builder.AddVertexDataType("User");
    builder.AddVertexDataType("Weibo");
    builder.AddEdgeDataType("UserWeibo");
    builder.AddEdgeDataType("UserInfluence");

    cerr << "Loading userIDName.txt..." << endl;
    ifstream user_file("userIDName.txt");
    string user_input;
    int i = 0;
    map<string, int> userid;
    while (getline(user_file, user_input)) {
        vector<string> inputs = split(user_input, '\t');
        User user;
        /*user.domain = inputs[1];
        user.avatar_large = inputs[2];
        user.bi_followers_count = convert_to_int(inputs[3]);
        user.block_word = convert_to_int(inputs[4]);
        user.star = convert_to_int(inputs[5]);
        user.id = inputs[6];
        user.city = convert_to_int(inputs[7]);
        user.verified = convert_to_bool(inputs[8]);
        //user.follow_me = convert_to_bool(inputs[9]);
        //user.verified_reason = inputs[10];
        user.followers_count = convert_to_int(inputs[11]);
        user.location = inputs[12];
        user.mbtype = convert_to_int(inputs[13]);
        user.profile_url = inputs[14];
        user.province = convert_to_int(inputs[15]);
        user.statuses_count = convert_to_int(inputs[16]);
        user.description = inputs[17];
        user.friends_count = convert_to_int(inputs[18]);
        //user.online_status = convert_to_int(inputs[19]);
        user.mbrank = convert_to_int(inputs[20]);
        //user.idstr = inputs[21];
        user.profile_image_url = inputs[22];
        //user.allow_all_act_msg = convert_to_bool(inputs[23]);
        //user.allow_all_comment = convert_to_bool(inputs[24]);
        //user.geo_enabled = convert_to_bool(inputs[25]);
        user.name = inputs[26];
        user.lang = inputs[27];
        user.weihao = inputs[28];
        user.remark = inputs[29];
        user.favourites_count = convert_to_int(inputs[30]);
        user.screen_name = inputs[31];
        user.url = inputs[32];
        user.gender = inputs[33];
        user.created_at = inputs[34];
        //user.verified_type = inputs[35];
        //user.following = convert_to_bool(inputs[36]);*/
		if (inputs.size() != 3) continue;
        user.id = inputs[0];
        user.name = inputs[1];
        user.followers_count = convert_to_int(inputs[2]);
        userid[user.id] = i;
        builder.AddVertex(USER_BASE + i++, user, "User");
    }
    user_file.close();

    cerr << "Loading Weibo data ..." << endl;
    int j = 0;
    for (i = 0; i < 10; i++) {
        string weibo_input;
        char id = '0' + i;
        string path = "mongodbdata/temp";
        path += id;
        path += ".txt";
        cout << "Loading file: " <<  path << endl;
        ifstream weibo_file(path);
        while (getline(weibo_file, weibo_input)) {
            vector<string> inputs = split(weibo_input, '\t');
            Weibo weibo;
            weibo.user_id = inputs[0];
            weibo.created_at = inputs[1];
            weibo.text = inputs[2];
            weibo.reposts_count = convert_to_int(inputs[3]);
            weibo.comments_count = convert_to_int(inputs[4]);
            builder.AddVertex (WEIBO_BASE + j, weibo, "Weibo");
            builder.AddEdge(USER_BASE + userid[inputs[0]], WEIBO_BASE + j++, UserWeibo(), "UserWeibo");
        }
        weibo_file.close();
    }
    
    cerr << "Loading user influence data..." << endl;
    ifstream user2user("User2User.txt");
    string influence;
    while (getline(user2user, influence)) {
        vector<string> inputs = split(influence, '\t');
        UserInfluence ui;
        ui.weight = convert_to_int(inputs[2]);
        builder.AddEdge(USER_BASE + userid[inputs[0]], USER_BASE + userid[inputs[1]], ui, "UserInfluence");
    }
    user2user.close();
    
    cerr << "Saving graph weibo..." << endl;
    builder.Save("weibo");

    return 0;
}
