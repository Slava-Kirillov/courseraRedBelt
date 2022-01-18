#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>

#include "profile.h"
#include "sync.h"

using namespace std;

class InvertedIndex {
public:

    void Add(string &document);

    vector<pair<size_t, size_t>> const &Lookup(const string_view &word) const;

    const string &GetDocument(size_t id) const {
        return docs[id];
    }

    size_t getDocsCount() const {
        return docs.size();
    }

private:
    map<string, vector<pair<size_t, size_t>>, less<>> index;
    vector<string> docs;
};

class SearchServer {
public:
    SearchServer() = default;

    explicit SearchServer(istream &document_input);

    void UpdateDocumentBase(istream &document_input);

    void AddQueriesStream(istream &query_input, ostream &search_results_output);

private:
    Synchronized<InvertedIndex> index;
};
