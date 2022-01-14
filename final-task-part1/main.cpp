#include "search_server.h"
#include "parse.h"
#include "test_runner.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <thread>
#include "profile.h"
#include "add_data.h"

using namespace std;

void TestFunctionality(
        const vector<string> &docs,
        const vector<string> &queries,
        const vector<string> &expected
) {
    istringstream docs_input(Join('\n', docs));
    istringstream queries_input(Join('\n', queries));

    SearchServer srv;
    srv.UpdateDocumentBase(docs_input);
    ostringstream queries_output;
    srv.AddQueriesStream(queries_input, queries_output);

    const string result = queries_output.str();
    const auto lines = SplitBy(Strip(result), '\n');
    ASSERT_EQUAL(lines.size(), expected.size());
    for (size_t i = 0; i < lines.size(); ++i) {
        ASSERT_EQUAL(lines[i], expected[i]);
    }
}

void TestSerpFormat() {
    const vector<string> docs = {
            "london is the capital of great britain",
            "i am travelling down the river"
    };
    const vector<string> queries = {"london", "the"};
    const vector<string> expected = {
            "london: {docid: 0, hitcount: 1}",
            Join(' ', vector{
                    "the:",
                    "{docid: 0, hitcount: 1}",
                    "{docid: 1, hitcount: 1}"
            })
    };

    TestFunctionality(docs, queries, expected);
}

void TestTop5() {
    const vector<string> docs = {
            "milk a",
            "milk b",
            "milk c",
            "milk d",
            "milk e",
            "milk f",
            "milk g",
            "water a",
            "water b",
            "fire and earth"
    };

    const vector<string> queries = {"milk", "water", "rock"};
    const vector<string> expected = {
            Join(' ', vector{
                    "milk:",
                    "{docid: 0, hitcount: 1}",
                    "{docid: 1, hitcount: 1}",
                    "{docid: 2, hitcount: 1}",
                    "{docid: 3, hitcount: 1}",
                    "{docid: 4, hitcount: 1}"
            }),
            Join(' ', vector{
                    "water:",
                    "{docid: 7, hitcount: 1}",
                    "{docid: 8, hitcount: 1}",
            }),
            "rock:",
    };
    TestFunctionality(docs, queries, expected);
}

void TestHitcount() {
    const vector<string> docs = {
            "the river goes through the entire city there is a house near it",
            "the wall",
            "walle",
            "is is is is",
    };
    const vector<string> queries = {"the", "wall", "all", "is", "the is"};
    const vector<string> expected = {
            Join(' ', vector{
                    "the:",
                    "{docid: 0, hitcount: 2}",
                    "{docid: 1, hitcount: 1}",
            }),
            "wall: {docid: 1, hitcount: 1}",
            "all:",
            Join(' ', vector{
                    "is:",
                    "{docid: 3, hitcount: 4}",
                    "{docid: 0, hitcount: 1}",
            }),
            Join(' ', vector{
                    "the is:",
                    "{docid: 3, hitcount: 4}",
                    "{docid: 0, hitcount: 3}",
                    "{docid: 1, hitcount: 1}",
            }),
    };
    TestFunctionality(docs, queries, expected);
}

void TestRanking() {
    const vector<string> docs = {
            "london is the capital of great britain",
            "paris is the capital of france",
            "berlin is the capital of germany",
            "rome is the capital of italy",
            "madrid is the capital of spain",
            "lisboa is the capital of portugal",
            "bern is the capital of switzerland",
            "moscow is the capital of russia",
            "kiev is the capital of ukraine",
            "minsk is the capital of belarus",
            "astana is the capital of kazakhstan",
            "beijing is the capital of china",
            "tokyo is the capital of japan",
            "bangkok is the capital of thailand",
            "welcome to moscow the capital of russia the third rome",
            "amsterdam is the capital of netherlands",
            "helsinki is the capital of finland",
            "oslo is the capital of norway",
            "stockgolm is the capital of sweden",
            "riga is the capital of latvia",
            "tallin is the capital of estonia",
            "warsaw is the capital of poland",
    };

    const vector<string> queries = {"moscow is the capital of russia"};
    const vector<string> expected = {
            Join(' ', vector{
                    "moscow is the capital of russia:",
                    "{docid: 7, hitcount: 6}",
                    "{docid: 14, hitcount: 6}",
                    "{docid: 0, hitcount: 4}",
                    "{docid: 1, hitcount: 4}",
                    "{docid: 2, hitcount: 4}",
            })
    };
    TestFunctionality(docs, queries, expected);
}

void TestBasicSearch() {
    const vector<string> docs = {
            "we are ready to go",
            "come on everybody shake you hands",
            "i love this game",
            "just like exception safety is not about writing try catch everywhere in your code move semantics are not about typing double ampersand everywhere in your code",
            "daddy daddy daddy dad dad dad",
            "tell me the meaning of being lonely",
            "just keep track of it",
            "how hard could it be",
            "it is going to be legen wait for it dary legendary",
            "we dont need no education"
    };

    vector<string> docs_2;
    for (int i = 0; i < 5; i++) {
        docs_2.insert(docs_2.end(), docs_1.begin(), docs_1.end());
    }

//    cout << "Docs: " << docs_2.size() << endl;

    const vector<string> queries = {
            "we need some help",
            "it",
            "i love this game",
            "tell me why",
            "dislike",
            "about"
    };

    const vector<string> queries_1 = {
            "we need some help need push size map final const",
            "it",
            "i love this game",
            "tell me why",
            "dislike",
            "about",
            "ZZZ"
    };

    vector<string> queries_2;
    for (int i = 0; i < 3000; i++) {
        queries_2.insert(queries_2.end(), queries_1.begin(), queries_1.end());
    }

//    cout << "Queries: " << queries_2.size() << endl;


    const vector<string> expected = {
            Join(' ', vector{
                    "we need some help:",
                    "{docid: 9, hitcount: 2}",
                    "{docid: 0, hitcount: 1}"
            }),
            Join(' ', vector{
                    "it:",
                    "{docid: 8, hitcount: 2}",
                    "{docid: 6, hitcount: 1}",
                    "{docid: 7, hitcount: 1}",
            }),
            "i love this game: {docid: 2, hitcount: 4}",
            "tell me why: {docid: 5, hitcount: 2}",
            "dislike:",
            "about: {docid: 3, hitcount: 2}",
    };
    TestFunctionality(docs_2, queries_2, expected);
//    TestFunctionality(docs, queries, expected);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSerpFormat);
    RUN_TEST(tr, TestTop5);
    RUN_TEST(tr, TestHitcount);
    RUN_TEST(tr, TestRanking);
    LOG_DURATION("Total")
    RUN_TEST(tr, TestBasicSearch);

//    string str_1 = "Test String";
//    string str_2 = "String Test";
//
//    vector<string_view> vec_1 = SplitIntoWordsView(str_1);
//    vector<string_view> vec_2 = SplitIntoWordsView(str_2);
//    map<string_view, string> m;
//
//    m[vec_1[0]] = "map string";
//
//    auto it = m.find(vec_2[1]);
//    if (it != m.end()) {
//        cout << (*it).second << endl;
//    }

}
