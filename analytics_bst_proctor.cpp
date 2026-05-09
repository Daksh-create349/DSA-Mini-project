// ============================================================
// analytics_bst_proctor.cpp
// P4: BST by Roll No + Auto Proctoring + Performance Analytics
// ============================================================
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <iomanip>
#include <stdexcept>
using namespace std;

// ─────────────────────────────────────────────
// BST Node
// ─────────────────────────────────────────────
struct BSTNode {
    int      rollNo;
    string   name;
    double   score;
    int      examId;
    BSTNode* left;
    BSTNode* right;

    BSTNode(int r, string n, double s, int e)
        : rollNo(r), name(n), score(s), examId(e),
          left(nullptr), right(nullptr) {}
};

// ─────────────────────────────────────────────
// BST — keyed by roll number
// ─────────────────────────────────────────────
class StudentBST {
    BSTNode* root;

    BSTNode* insert(BSTNode* node, int roll, string name, double score, int examId) {
        if (!node) return new BSTNode(roll, name, score, examId);
        if      (roll < node->rollNo) node->left  = insert(node->left,  roll, name, score, examId);
        else if (roll > node->rollNo) node->right = insert(node->right, roll, name, score, examId);
        else { node->score = score; node->examId = examId; }
        return node;
    }

    BSTNode* search(BSTNode* node, int roll) {
        if (!node || node->rollNo == roll) return node;
        if (roll < node->rollNo) return search(node->left, roll);
        return search(node->right, roll);
    }

    void inorder(BSTNode* node, vector<BSTNode*>& result) {
        if (!node) return;
        inorder(node->left, result);
        result.push_back(node);
        inorder(node->right, result);
    }

    void rangeQuery(BSTNode* node, int lo, int hi, vector<BSTNode*>& result) {
        if (!node) return;
        if (node->rollNo > lo) rangeQuery(node->left, lo, hi, result);
        if (node->rollNo >= lo && node->rollNo <= hi) result.push_back(node);
        if (node->rollNo < hi) rangeQuery(node->right, lo, hi, result);
    }

    void destroy(BSTNode* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    StudentBST() : root(nullptr) {}
    StudentBST(const StudentBST&) = delete;
    StudentBST& operator=(const StudentBST&) = delete;
    ~StudentBST() { destroy(root); }

    void insert(int roll, string name, double score, int examId) {
        if (roll <= 0) throw invalid_argument("Roll number must be positive.");
        if (name.empty()) throw invalid_argument("Student name cannot be empty.");
        if (examId <= 0) throw invalid_argument("Exam id must be positive.");
        root = insert(root, roll, name, score, examId);
    }

    BSTNode* search(int roll) { return search(root, roll); }

    vector<BSTNode*> getAllSorted() {
        vector<BSTNode*> result;
        inorder(root, result);
        return result;
    }

    vector<BSTNode*> getRangeByRoll(int lo, int hi) {
        vector<BSTNode*> result;
        rangeQuery(root, lo, hi, result);
        return result;
    }

    void displayAll() {
        auto all = getAllSorted();
        cout << "\n===== All Students (BST Inorder by RollNo) =====\n";
        cout << left << setw(8) << "  Roll" << setw(22) << "  Name"
             << setw(8) << "Score" << "ExamID\n";
        cout << "  " << string(44, '-') << "\n";
        for (auto* n : all)
            cout << "  " << left << setw(8) << n->rollNo
                 << setw(22) << n->name
                 << fixed << setprecision(2) << setw(8) << n->score
                 << n->examId << "\n";
        if (all.empty()) cout << "  (no records)\n";
    }

    void displayResult(int roll) {
        BSTNode* n = search(roll);
        if (!n) { cout << "  [BST] Roll " << roll << " not found.\n"; return; }
        cout << "\n  [BST] Found Roll " << roll << ":\n"
             << "    Name  : " << n->name  << "\n"
             << "    Score : " << fixed << setprecision(2) << n->score << "\n"
             << "    ExamID: " << n->examId << "\n";
    }
};

// ─────────────────────────────────────────────
// Proctoring Event Types
// ─────────────────────────────────────────────
enum class ProctorEvent {
    TIME_GAP, RAPID_CLICK, COPY_PASTE, TAB_SWITCH, FACE_ABSENT
};

inline string eventName(ProctorEvent e) {
    switch (e) {
        case ProctorEvent::TIME_GAP:    return "TIME_GAP";
        case ProctorEvent::RAPID_CLICK: return "RAPID_CLICK";
        case ProctorEvent::COPY_PASTE:  return "COPY_PASTE";
        case ProctorEvent::TAB_SWITCH:  return "TAB_SWITCH";
        case ProctorEvent::FACE_ABSENT: return "FACE_ABSENT";
        default: return "UNKNOWN";
    }
}

struct ProctorAlert {
    int         studentId;
    string      studentName;
    ProctorEvent type;
    string      detail;
    long long   timestamp;
};

// ─────────────────────────────────────────────
// Auto Proctoring Engine
// ─────────────────────────────────────────────
class AutoProctor {
    vector<ProctorAlert> alerts;
    static const int TIME_GAP_THRESHOLD = 120;
    static const int RAPID_CLICK_MS     = 1500;

    void logAlert(ProctorAlert a) {
        alerts.push_back(a);
        cout << "  [ALERT] " << a.studentName
             << " | " << eventName(a.type)
             << " | " << a.detail << "\n";
    }

public:
    void checkTimeGap(int sid, const string& name,
                      long long prev, long long curr) {
        long long gap = curr - prev;
        if (gap > TIME_GAP_THRESHOLD)
            logAlert({sid, name, ProctorEvent::TIME_GAP,
                      "Gap of " + to_string(gap) + "s between answers", curr});
    }

    void checkRapidClicking(int sid, const string& name,
                             const vector<long long>& timestamps) {
        int rapid = 0;
        for (int i = 1; i < (int)timestamps.size(); i++)
            if (timestamps[i] - timestamps[i-1] < RAPID_CLICK_MS) rapid++;
        if (rapid >= 3)
            logAlert({sid, name, ProctorEvent::RAPID_CLICK,
                      to_string(rapid) + " rapid clicks detected",
                      timestamps.back()});
    }

    void reportCopyPaste(int sid, const string& name, long long ts) {
        logAlert({sid, name, ProctorEvent::COPY_PASTE,
                  "Copy-paste action detected", ts});
    }

    void reportTabSwitch(int sid, const string& name, long long ts) {
        logAlert({sid, name, ProctorEvent::TAB_SWITCH,
                  "Browser tab switch detected", ts});
    }

    void reportFaceAbsent(int sid, const string& name, long long ts) {
        logAlert({sid, name, ProctorEvent::FACE_ABSENT,
                  "Face not visible on webcam", ts});
    }

    vector<ProctorAlert> getAlertsForStudent(int sid) {
        vector<ProctorAlert> result;
        for (auto& a : alerts)
            if (a.studentId == sid) result.push_back(a);
        return result;
    }

    bool isSuspicious(int sid) {
        return (int)getAlertsForStudent(sid).size() >= 3;
    }

    void displayAllAlerts() {
        cout << "\n===== All Proctoring Alerts (" << alerts.size() << ") =====\n";
        if (alerts.empty()) { cout << "  (none)\n"; return; }
        for (auto& a : alerts)
            cout << "  ID:" << a.studentId
                 << " | " << left << setw(13) << eventName(a.type)
                 << " | " << a.detail << "\n";
    }

    void generateReport(int sid, const string& name) {
        auto sa = getAlertsForStudent(sid);
        cout << "\n===== Proctoring Report =====\n"
             << "  Student : " << name << " (ID:" << sid << ")\n"
             << "  Alerts  : " << sa.size() << "\n"
             << "  Status  : " << (isSuspicious(sid) ? "*** SUSPICIOUS ***" : "CLEAN") << "\n";
        for (auto& a : sa)
            cout << "    - " << eventName(a.type) << ": " << a.detail << "\n";
    }
};

// ─────────────────────────────────────────────
// Performance Analytics
// ─────────────────────────────────────────────
struct PerformanceStats {
    double average, highest, lowest, stdDev;
    unordered_map<string, double> topicWiseAvg;
};

class PerformanceAnalytics {
public:
    static PerformanceStats compute(const vector<double>& scores) {
        PerformanceStats ps = {0, 0, 0, 0, {}};
        if (scores.empty()) return ps;
        double sum = 0;
        ps.highest = ps.lowest = scores[0];
        for (double s : scores) {
            sum += s;
            if (s > ps.highest) ps.highest = s;
            if (s < ps.lowest)  ps.lowest  = s;
        }
        ps.average = sum / scores.size();
        double var = 0;
        for (double s : scores) var += (s - ps.average) * (s - ps.average);
        ps.stdDev = sqrt(var / scores.size());
        return ps;
    }

    static unordered_map<string, double>
    topicWiseAverage(unordered_map<string, vector<double>>& topicScores) {
        unordered_map<string, double> result;
        for (auto& entry : topicScores) {
            const string& topic = entry.first;
            vector<double>& scores = entry.second;
            if (scores.empty()) continue;
            double sum = 0;
            for (double s : scores) sum += s;
            result[topic] = sum / scores.size();
        }
        return result;
    }

    static void displayStats(const PerformanceStats& ps, const string& label = "Exam") {
        cout << "\n===== Performance Analytics: " << label << " =====\n"
             << fixed << setprecision(2)
             << "  Average : " << ps.average << "\n"
             << "  Highest : " << ps.highest << "\n"
             << "  Lowest  : " << ps.lowest  << "\n"
             << "  Std Dev : " << ps.stdDev  << "\n";
        if (!ps.topicWiseAvg.empty()) {
            cout << "  Topic-wise:\n";
            for (auto& entry : ps.topicWiseAvg)
                cout << "    " << left << setw(12) << entry.first << ": " << entry.second << "\n";
        }
    }
};
