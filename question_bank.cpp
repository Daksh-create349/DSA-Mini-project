// ============================================================
// question_bank.cpp
// P1: Question Bank — Linked List + Trie Keyword Search
// ============================================================
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <stdexcept>
using namespace std;

// ─────────────────────────────────────────────
// Question Node
// ─────────────────────────────────────────────
#ifndef QUESTION_MODEL_DEFINED
#define QUESTION_MODEL_DEFINED
struct Question {
    int            id;
    string         text;
    string         topic;
    int            difficulty;   // 1=easy … 5=hard
    vector<string> options;      // exactly 4
    int            answer;       // correct index 0-3
    vector<string> keywords;
    Question*      next;

    Question(int id, string text, string topic, int diff,
             vector<string> opts, int ans, vector<string> kws)
        : id(id), text(text), topic(topic), difficulty(diff),
          options(opts), answer(ans), keywords(kws), next(nullptr) {}
};
#endif

// ─────────────────────────────────────────────
// Trie Node
// ─────────────────────────────────────────────
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    bool        isEnd = false;
    vector<int> questionIds;
};

// ─────────────────────────────────────────────
// Trie
// ─────────────────────────────────────────────
class Trie {
    TrieNode* root;

    void collectAll(TrieNode* node, vector<int>& result) const {
        if (!node) return;
        if (node->isEnd)
            for (int id : node->questionIds)
                result.push_back(id);
        for (auto& entry : node->children)
            collectAll(entry.second, result);
    }

    void destroy(TrieNode* node) {
        if (!node) return;
        for (auto& entry : node->children)
            destroy(entry.second);
        delete node;
    }

public:
    Trie() { root = new TrieNode(); }
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;
    ~Trie() { destroy(root); }

    void insert(const string& word, int qId) {
        TrieNode* cur = root;
        for (char c : word) {
            c = (char)tolower((unsigned char)c);
            if (!cur->children.count(c))
                cur->children[c] = new TrieNode();
            cur = cur->children[c];
        }
        cur->isEnd = true;
        cur->questionIds.push_back(qId);
    }

    vector<int> search(const string& prefix) const {
        TrieNode* cur = root;
        for (char c : prefix) {
            c = (char)tolower((unsigned char)c);
            if (!cur->children.count(c)) return {};
            cur = cur->children[c];
        }
        vector<int> result;
        collectAll(cur, result);
        return result;
    }
};

// ─────────────────────────────────────────────
// Question Bank (Linked List + Trie)
// ─────────────────────────────────────────────
class QuestionBank {
    Question* head;
    int       size;
    Trie      trie;
    int       nextId;

public:
    QuestionBank() : head(nullptr), size(0), nextId(1) {}
    QuestionBank(const QuestionBank&) = delete;
    QuestionBank& operator=(const QuestionBank&) = delete;

    void addQuestion(string text, string topic, int diff,
                     vector<string> opts, int ans, vector<string> kws) {
        if (text.empty()) throw invalid_argument("Question text cannot be empty.");
        if (topic.empty()) throw invalid_argument("Question topic cannot be empty.");
        if (diff < 1 || diff > 5) throw invalid_argument("Difficulty must be between 1 and 5.");
        if (opts.size() != 4) throw invalid_argument("Each question must have exactly 4 options.");
        if (ans < 0 || ans >= (int)opts.size()) throw invalid_argument("Answer index is out of range.");

        Question* q = new Question(nextId++, text, topic, diff, opts, ans, kws);
        for (auto& kw : kws) trie.insert(kw, q->id);
        trie.insert(topic, q->id);
        if (!head) { head = q; }
        else {
            Question* cur = head;
            while (cur->next) cur = cur->next;
            cur->next = q;
        }
        size++;
    }

    vector<Question*> getByTopic(const string& topic) const {
        vector<Question*> result;
        Question* cur = head;
        while (cur) {
            if (cur->topic == topic) result.push_back(cur);
            cur = cur->next;
        }
        return result;
    }

    vector<Question*> getByDifficulty(int diff) const {
        vector<Question*> result;
        Question* cur = head;
        while (cur) {
            if (cur->difficulty == diff) result.push_back(cur);
            cur = cur->next;
        }
        return result;
    }

    vector<Question*> searchByKeyword(const string& prefix) const {
        vector<int> ids = trie.search(prefix);
        // deduplicate
        vector<int> seen;
        vector<Question*> result;
        for (int id : ids) {
            if (find(seen.begin(), seen.end(), id) == seen.end()) {
                seen.push_back(id);
                Question* q = findById(id);
                if (q) result.push_back(q);
            }
        }
        return result;
    }

    Question* findById(int id) const {
        Question* cur = head;
        while (cur) {
            if (cur->id == id) return cur;
            cur = cur->next;
        }
        return nullptr;
    }

    vector<Question*> getAllQuestions() const {
        vector<Question*> result;
        Question* cur = head;
        while (cur) { result.push_back(cur); cur = cur->next; }
        return result;
    }

    int getSize() const { return size; }

    void displayAll() const {
        Question* cur = head;
        cout << "\n===== Question Bank (" << size << " questions) =====\n";
        while (cur) {
            cout << "  [Q" << cur->id << "] [" << cur->topic
                 << "] [Diff:" << cur->difficulty << "] "
                 << cur->text << "\n";
            cur = cur->next;
        }
    }

    ~QuestionBank() {
        Question* cur = head;
        while (cur) { Question* tmp = cur; cur = cur->next; delete tmp; }
    }
};
