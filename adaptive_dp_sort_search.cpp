// ============================================================
// adaptive_dp_sort_search.cpp
// P3: DP Adaptive Selection + Merge Sort + Binary Search
// ============================================================
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
using namespace std;

// Forward declare Question so this file knows about it
#ifndef QUESTION_MODEL_DEFINED
#define QUESTION_MODEL_DEFINED
struct Question {
    int            id;
    string         text;
    string         topic;
    int            difficulty;
    vector<string> options;
    int            answer;
    vector<string> keywords;
    Question*      next;
};
#endif

// ─────────────────────────────────────────────
// DP Adaptive Exam Selector
// ─────────────────────────────────────────────
class AdaptiveExam {
public:
    // DP Knapsack: maximize topic coverage within time limit
    // Only picks questions within difficulty ±1 of student level
    vector<Question*> selectQuestions(vector<Question*>& pool,
                                      int timeLimitSecs,
                                      int studentLevel,
                                      int secsPerQuestion = 60) {
        vector<Question*> eligible;
        if (timeLimitSecs <= 0 || secsPerQuestion <= 0) return {};
        for (auto* q : pool)
            if (q && abs(q->difficulty - studentLevel) <= 1)
                eligible.push_back(q);

        int n = (int)eligible.size();
        if (n == 0) return {};

        unordered_map<string, int> topicCount;
        for (auto* q : eligible) topicCount[q->topic]++;

        vector<int> value(n), weight(n);
        for (int i = 0; i < n; i++) {
            int rarity = max(1, 10 - topicCount[eligible[i]->topic]);
            value[i]  = eligible[i]->difficulty * 10 + rarity;
            weight[i] = secsPerQuestion;
        }

        int W = timeLimitSecs;
        vector<vector<int>> dp(n + 1, vector<int>(W + 1, 0));
        for (int i = 1; i <= n; i++) {
            for (int w = 0; w <= W; w++) {
                dp[i][w] = dp[i-1][w];
                if (weight[i-1] <= w)
                    dp[i][w] = max(dp[i][w], dp[i-1][w - weight[i-1]] + value[i-1]);
            }
        }

        vector<Question*> selected;
        int w = W;
        for (int i = n; i >= 1; i--) {
            if (dp[i][w] != dp[i-1][w]) {
                selected.push_back(eligible[i-1]);
                w -= weight[i-1];
            }
        }
        reverse(selected.begin(), selected.end());

        cout << "\n  [DP] Selected " << selected.size() << " questions"
             << " | studentLevel=" << studentLevel
             << " | timeLimit=" << timeLimitSecs << "s\n";
        return selected;
    }

    // Adaptive difficulty: adjust after each answer
    int adjustDifficulty(int currentDiff, bool correct,
                         int correctStreak, int wrongStreak) {
        int newDiff = max(1, min(5, currentDiff));
        if (correct  && correctStreak >= 2 && newDiff < 5) newDiff++;
        if (!correct && wrongStreak   >= 2 && newDiff > 1) newDiff--;
        if (newDiff != currentDiff)
            cout << "  [Adaptive] Difficulty: " << currentDiff << " -> " << newDiff << "\n";
        return newDiff;
    }
};

// ─────────────────────────────────────────────
// Student Record (for sorting / ranking)
// ─────────────────────────────────────────────
struct StudentRecord {
    int    rollNo;
    string name;
    double score;
    int    attemptNumber;
};

// ─────────────────────────────────────────────
// Merge Sort — stable, descending score
// ─────────────────────────────────────────────
class MergeSort {
    static void merge(vector<StudentRecord>& arr, int l, int mid, int r) {
        vector<StudentRecord> tmp;
        int i = l, j = mid + 1;
        while (i <= mid && j <= r) {
            if (arr[i].score > arr[j].score ||
               (arr[i].score == arr[j].score && arr[i].rollNo <= arr[j].rollNo))
                tmp.push_back(arr[i++]);
            else
                tmp.push_back(arr[j++]);
        }
        while (i <= mid) tmp.push_back(arr[i++]);
        while (j <= r)   tmp.push_back(arr[j++]);
        for (int k = l; k <= r; k++) arr[k] = tmp[k - l];
    }

    static void mergeSort(vector<StudentRecord>& arr, int l, int r) {
        if (l >= r) return;
        int mid = (l + r) / 2;
        mergeSort(arr, l, mid);
        mergeSort(arr, mid + 1, r);
        merge(arr, l, mid, r);
    }

public:
    static void sort(vector<StudentRecord>& arr) {
        if (arr.size() <= 1) return;
        mergeSort(arr, 0, (int)arr.size() - 1);
    }

    // Dedup: keep latest attempt per rollNo
    static vector<StudentRecord> dedup(const vector<StudentRecord>& arr) {
        unordered_map<int, StudentRecord> best;
        for (const auto& s : arr)
            if (!best.count(s.rollNo) || s.attemptNumber > best[s.rollNo].attemptNumber)
                best[s.rollNo] = s;
        vector<StudentRecord> result;
        for (auto& entry : best) result.push_back(entry.second);
        sort(result);
        return result;
    }

    static void display(const vector<StudentRecord>& arr) {
        cout << "\n===== Student Ranking =====\n";
        cout << "  Rank | Roll  | Name                | Score\n";
        cout << "  -----|-------|---------------------|--------\n";
        for (int i = 0; i < (int)arr.size(); i++)
            printf("  %-5d| %-6d| %-21s| %.2f\n",
                   i+1, arr[i].rollNo, arr[i].name.c_str(), arr[i].score);
    }
};

// ─────────────────────────────────────────────
// Binary Search — on topic-sorted questions
// ─────────────────────────────────────────────
class BinarySearch {
public:
    static vector<Question*> searchByTopic(vector<Question*>& sorted,
                                           const string& target) {
        int lo = 0, hi = (int)sorted.size() - 1, found = -1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if      (sorted[mid]->topic == target) { found = mid; break; }
            else if (sorted[mid]->topic  < target)   lo = mid + 1;
            else                                      hi = mid - 1;
        }
        if (found == -1) return {};

        int left = found, right = found;
        while (left  > 0 && sorted[left-1]->topic  == target) left--;
        while (right < (int)sorted.size()-1 && sorted[right+1]->topic == target) right++;

        vector<Question*> result;
        for (int i = left; i <= right; i++) result.push_back(sorted[i]);
        return result;
    }

    static vector<Question*> sortAndSearch(vector<Question*> questions,
                                           const string& topic) {
        questions.erase(remove(questions.begin(), questions.end(), nullptr), questions.end());
        sort(questions.begin(), questions.end(),
             [](Question* a, Question* b){ return a->topic < b->topic; });
        return searchByTopic(questions, topic);
    }
};
