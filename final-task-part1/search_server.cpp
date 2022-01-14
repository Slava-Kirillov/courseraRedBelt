#include "search_server.h"
#include "iterator_range.h"
#include "profile.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <array>

static const vector<pair<size_t, size_t>> empty_vec;

vector<string_view> SplitIntoWordsView(string_view str) {
    vector<string_view> result;
    while (true) {
        size_t space = str.find(' ');
        string_view temp = str.substr(0, space);
        if (!temp.empty()) {
            result.push_back(temp);
        }
        if (space == std::string_view::npos) {
            break;
        } else {
            str.remove_prefix(space + 1);
        }
    }
    return move(result);
}

SearchServer::SearchServer(istream &document_input) {
    UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream &document_input) {
    InvertedIndex new_index;

    for (string current_document; getline(document_input, current_document);) {
        new_index.Add(current_document);
    }

    index = move(new_index);
}

void SearchServer::AddQueriesStream(istream &query_input, ostream &search_results_output) {

    vector<pair<size_t, size_t>> search_results(index.getDocsCount(), make_pair(50001, 0));
    for (string current_query; getline(query_input, current_query);) {

        vector<string_view> words = SplitIntoWordsView(current_query);
        for (string_view word: words) {
            for (pair<size_t, size_t> doc: index.Lookup(word)) {
                search_results[doc.first].first = doc.first;
                search_results[doc.first].second += doc.second;
            }
        }

        partial_sort(
                search_results.begin(),
                Head(search_results, 5).end(),
                search_results.end(),
                [](pair<size_t, size_t> &lhs, pair<size_t, size_t> &rhs) {
                    return (lhs.second > rhs.second) || (lhs.second == rhs.second && lhs.first < rhs.first);
                }
        );

        search_results_output << current_query << ':';
        for (auto &[docid, hitcount]: Head(search_results, 5)) {
            if (hitcount > 0) {
                search_results_output << " {docid: " << docid << ", hitcount: " << hitcount << '}';
            }
        }

        search_results_output << endl;

        search_results.assign(index.getDocsCount(), make_pair(50001, 0));
    }
}

void InvertedIndex::Add(string &document) {
    docs.push_back(move(document));

    for (string_view& word: SplitIntoWordsView(docs[docs.size() - 1])) {
        vector<pair<size_t, size_t>> & vec = index[{word.begin(), word.end()}];
        if (vec.empty() || vec.back().first != (docs.size() - 1)) {
            vec.emplace_back(docs.size() - 1, 1);
        } else {
            vec.back().second++;
        }
    }
}

const vector<pair<size_t, size_t>> &InvertedIndex::Lookup(const string_view &word) const {
    if (auto it = index.find(word); it != index.end()) {
        return it->second;
    } else {
        return empty_vec;
    }
}
