#include "aminerdata.hpp"

int main() {
    AMinerData aminer("aminer");
    auto result = aminer.searchPublications("data mining");
    cout << "Publication search result: " << endl;
    for (auto& i : result) {
        cout << "search result: " << i.docId << ", score: " << i.score << endl;
        auto p = aminer.get<Publication>(i.docId);
        cout << "title: " << p.title << endl;
    }
    auto authors = aminer.searchAuthors("data mining");
    cout << "Author search result: " << endl;
    for (auto& i : authors) {
        cout << "search result: " << i.docId << ", score: " << i.score << endl;
        auto p = aminer.get<Author>(i.docId);
        cout << "name: " << p.names[0] << endl;
    }
}
