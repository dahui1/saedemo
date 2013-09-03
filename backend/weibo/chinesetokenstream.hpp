#pragma once
#include <iostream>
#include <fstream>
#include <set>

#include "glog/logging.h"
#include "ICTCLAS50.h"
#include "indexing/analyzer/ArnetAnalyzer.h"

DECLARE_string(ictclas_data);
DECLARE_string(stopwords);

class ChineseTokenizer :
    public TokenStream
{
public:
    ChineseTokenizer(void)
    {
    }
    ~ChineseTokenizer(void)
    {
    }

    bool isPunctuation(const std::string& s) {
        if (s.size() == 1 && ispunct(s[0]))
            return true;
        else if (s=="；"||s=="："||s=="“"||s=="”"||s=="‘"||s=="’"||s=="，"||s=="。"
            ||s=="《"||s=="》"||s=="？"||s=="、"||s=="·"||s=="！"||s=="…"||s=="（"||s=="）"
            ||s=="【"||s=="】")
            return true;
        return false;
    }

    std::vector<std::string> split(const std::string& s, char c) {
        std::vector<std::string> v;
        int last = 0;
        for (int i=0; i<s.size(); i++) {
            if (s[i] == c) {
                v.push_back(s.substr(last, i - last));
                last = i + 1;
            }
        }
        v.push_back(s.substr(last, s.size() - last));
        return v;
    }

    std::string filter(const std::string& s, const std::set<string>& stopwords) {
        auto words = split(s, ' ');
        string result = "";
        for (int i = 0; i < words.size(); i++) {
            std::string term = words[i];
            if (!isPunctuation(term)) {
                auto stop = stopwords.find(term);
                if (stop == stopwords.end()) {
                    result += term;
                    if (i != words.size() - 1) result += " ";
                }
            }
        }
        return result;
    }

    ChineseTokenizer(const char* in, const std::set<string>& stopwords) {
        offset = 0;
        const string res = filter(in, stopwords);
        ioBuffer = res;
        dataLen = ioBuffer.size();
    }

    ChineseTokenizer(const std::string& in) {
        offset = 0;
        ioBuffer = in;
        dataLen = ioBuffer.size();
    }

    bool next(Token& token)
    {
        int length = 0;
        int start = offset;
        wchar_t space = ' ';
        const char* cur_ptr = ioBuffer.c_str() + offset, *end_ptr = ioBuffer.c_str() + dataLen;
        while (cur_ptr!=end_ptr)
        {
            ++offset;
            if (*cur_ptr != ' ')
            {
                ++length;
            }
            else
            {
                if (length) break;
                start = offset;
            }
            ++cur_ptr;
        }

        if (length)
        {
            token.setTermText(ioBuffer.c_str(), start, length);
            token.setStartOffset(start);
            token.setEndOffset(start + length);
            return true;
        }
        else
        {
            return false;
        }
    }

    void reset(){
        offset = 0;
    };

private:

    int offset, dataLen;
    string ioBuffer;
};

struct ChineseTokenStream {
    ChineseTokenStream() {
        LOG(INFO) << "Initializing ICTCLAS, data: " << FLAGS_ictclas_data;
        if (!ICTCLAS_Init(FLAGS_ictclas_data.c_str())) {
            LOG(FATAL) << "ICTCLAS initilization failed!";
        }
        ICTCLAS_SetPOSmap(2);
        LOG(INFO) << "ICTCLAS initialized.";
        load_stop_words();
    }

    void load_stop_words() {
        LOG(INFO) << "loading stopword dic...";
        std::ifstream stopword_file(FLAGS_stopwords);
        std::string stopword_input;
        while (std::getline(stopword_file, stopword_input)) {
            stopwords.insert(stopword_input);
        }
        LOG(INFO) << stopwords.size() << " stopwords loaded.";
        stopword_file.close();
    }

    // TODO return unique_ptr
    TokenStream* operator() (const std::string& text) {
        std::unique_ptr<char[]> sRst(new char[text.size() * 6]);
        int nRstLen=0;
        nRstLen = ICTCLAS_ParagraphProcess(text.c_str(), text.size(), sRst.get(), CODE_TYPE_UNKNOWN, 0);
        return new ChineseTokenizer(sRst.get(), stopwords);
    }
private:
    std::set<std::string> stopwords;
};
