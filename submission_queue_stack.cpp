// ============================================================
// submission_queue_stack.cpp
// P2: Submission Queue (FIFO) + Answer Review Stack (LIFO)
// ============================================================
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
using namespace std;

// ─────────────────────────────────────────────
// Submission Node
// ─────────────────────────────────────────────
struct Submission {
    int         studentId;
    int         examId;
    string      studentName;
    vector<int> answers;
    long long   timestamp;
    string      source;
    bool        plagiarismFlag;
    Submission* next;

    Submission(int sid, int eid, string name, vector<int> ans, long long ts, string src)
        : studentId(sid), examId(eid), studentName(name),
          answers(ans), timestamp(ts), source(src),
          plagiarismFlag(false), next(nullptr) {}
};

// ─────────────────────────────────────────────
// Submission Queue — FIFO
// ─────────────────────────────────────────────
class SubmissionQueue {
    Submission* front;
    Submission* rear;
    int         count;

public:
    SubmissionQueue() : front(nullptr), rear(nullptr), count(0) {}
    SubmissionQueue(const SubmissionQueue&) = delete;
    SubmissionQueue& operator=(const SubmissionQueue&) = delete;

    void enqueue(int sid, int eid, string name, vector<int> ans, long long ts,
                 const string& source = "Manual Entry") {
        if (sid <= 0) throw invalid_argument("Student id must be positive.");
        if (eid <= 0) throw invalid_argument("Exam id must be positive.");
        if (name.empty()) throw invalid_argument("Student name cannot be empty.");
        Submission* s = new Submission(sid, eid, name, ans, ts, source);
        if (!rear) { front = rear = s; }
        else       { rear->next = s; rear = s; }
        count++;
        cout << "  [Queue] Enqueued — " << name
             << " (ExamID:" << eid << ", Source:" << source << ")\n";
    }

    Submission* dequeue() {
        if (!front) { cout << "  [Queue] Empty!\n"; return nullptr; }
        Submission* s = front;
        front = front->next;
        if (!front) rear = nullptr;
        count--;
        s->next = nullptr;
        return s;
    }

    Submission* peek() const { return front; }
    bool  isEmpty()    const { return front == nullptr; }
    int   getCount()   const { return count; }

    vector<Submission*> getAll() const {
        vector<Submission*> v;
        Submission* cur = front;
        while (cur) { v.push_back(cur); cur = cur->next; }
        return v;
    }

    int flagPlagiarismByHash() {
        unordered_map<string, int> seenAnswerPatterns;
        int flagged = 0;

        for (Submission* cur = front; cur; cur = cur->next) {
            string key;
            for (int answer : cur->answers)
                key += to_string(answer) + "#";

            if (seenAnswerPatterns.count(key)) {
                cur->plagiarismFlag = true;
                flagged++;
            } else {
                seenAnswerPatterns[key] = cur->studentId;
            }
        }

        cout << "  [Plagiarism] Flagged " << flagged
             << " duplicate answer pattern(s) using hash comparison.\n";
        return flagged;
    }

    void display() const {
        cout << "\n===== Submission Queue (" << count << " pending) =====\n";
        Submission* cur = front;
        int pos = 1;
        while (cur) {
            cout << "  " << pos++ << ". " << cur->studentName
                 << " | ID:" << cur->studentId
                 << " | ExamID:" << cur->examId
                 << " | Source:" << cur->source
                 << " | Answers: ";
            for (int a : cur->answers) cout << a << " ";
            cout << (cur->plagiarismFlag ? " [PLAGIARISM FLAG]" : "") << "\n";
            cur = cur->next;
        }
        if (count == 0) cout << "  (empty)\n";
    }

    ~SubmissionQueue() {
        while (front) { Submission* tmp = front; front = front->next; delete tmp; }
    }
};

// ─────────────────────────────────────────────
// Review Record
// ─────────────────────────────────────────────
struct ReviewRecord {
    int    questionId;
    int    previousAnswer;
    int    newAnswer;
    string note;
};

// ─────────────────────────────────────────────
// Answer Review Stack — LIFO
// ─────────────────────────────────────────────
class AnswerReviewStack {
    vector<ReviewRecord> stack;

public:
    void push(int qId, int prevAns, int newAns, const string& note = "") {
        stack.push_back({qId, prevAns, newAns, note});
        cout << "  [Stack] Recorded — Q" << qId
             << ": " << prevAns << " -> " << newAns << "\n";
    }

    ReviewRecord pop() {
        if (stack.empty()) {
            cout << "  [Stack] Nothing to undo!\n";
            return {-1, -1, -1, ""};
        }
        ReviewRecord r = stack.back();
        stack.pop_back();
        cout << "  [Stack] Undone — Q" << r.questionId
             << ": reverted " << r.newAnswer << " -> " << r.previousAnswer << "\n";
        return r;
    }

    ReviewRecord peek() const {
        if (stack.empty()) return {-1, -1, -1, ""};
        return stack.back();
    }

    bool isEmpty()  const { return stack.empty(); }
    int  getSize()  const { return (int)stack.size(); }

    void clear() {
        stack.clear();
        cout << "  [Stack] Cleared all review history.\n";
    }

    void displayHistory() const {
        cout << "\n===== Answer Review History (top -> bottom) =====\n";
        if (stack.empty()) { cout << "  (empty)\n"; return; }
        for (int i = (int)stack.size() - 1; i >= 0; --i) {
            auto& r = stack[i];
            cout << "  Q" << r.questionId
                 << " | prev=" << r.previousAnswer
                 << " | new="  << r.newAnswer;
            if (!r.note.empty()) cout << " | " << r.note;
            cout << "\n";
        }
    }
};
