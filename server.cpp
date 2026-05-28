// ============================================================
// server.cpp — HTTP Backend for Adaptive Exam Platform
// Wraps existing C++ modules as a REST API
// ============================================================
#include "httplib.h"
#include <sstream>
#include <mutex>
#include <chrono>
#include <limits>

// Include existing modules (same order as main.cpp)
#include "question_bank.cpp"
#include "submission_queue_stack.cpp"
#include "adaptive_dp_sort_search.cpp"
#include "analytics_bst_proctor.cpp"

using namespace std;
mutex g_mtx;
long long g_nextTs = 1000;

// ── Global State (same as main.cpp) ──
QuestionBank g_bank;
SubmissionQueue g_submissions;
AnswerReviewStack g_reviewStack;
StudentBST g_resultTree;
vector<StudentRecord> g_records;
AutoProctor g_proctor;
struct ExamActivity {
    string studentName;
    vector<long long> answerTimesSeconds;
    vector<long long> answerTimesMillis;
    bool timingAnalyzed = false;
};
unordered_map<int, ExamActivity> g_activityLog;

// Genuine per-topic performance tracking (same as main.cpp)
// Key: topic name, Value: vector of per-question scores (100=correct, 0=wrong)
unordered_map<string, vector<double>> g_topicScoresMap;

// ── Seed Data (from main.cpp) ──
void seedAll() {
    g_bank.addQuestion("What is the time complexity of binary search?", "Searching", 2,
        {"O(n)", "O(log n)", "O(n log n)", "O(1)"}, 1, {"binary", "search", "complexity"});
    g_bank.addQuestion("Which data structure follows FIFO?", "Queue", 1,
        {"Stack", "Queue", "Tree", "Graph"}, 1, {"fifo", "queue"});
    g_bank.addQuestion("Which traversal of BST gives sorted order?", "BST", 3,
        {"Preorder", "Postorder", "Inorder", "Level order"}, 2, {"bst", "tree", "inorder"});
    g_bank.addQuestion("Which algorithm is stable and divide-and-conquer?", "Sorting", 3,
        {"Quick Sort", "Merge Sort", "Selection Sort", "Heap Sort"}, 1, {"merge", "sort", "stable"});
    g_bank.addQuestion("Dynamic programming avoids repeated work using what?", "DP", 4,
        {"Recursion only", "Memoization", "Randomization", "Hash collision"}, 1, {"dp", "memoization", "optimal"});
    g_bank.addQuestion("A stack removes the most recently inserted item using which rule?", "Stack", 1,
        {"FIFO", "LIFO", "Priority", "Hashing"}, 1, {"stack", "lifo"});

    g_resultTree.insert(101, "Aarav", 86.0, 501);
    g_resultTree.insert(102, "Meera", 82.5, 501);
    g_resultTree.insert(103, "Kabir", 91.0, 501);
    g_records = {
        {101, "Aarav", 86.0, 1},
        {102, "Meera", 79.0, 1},
        {102, "Meera", 82.5, 2},
        {103, "Kabir", 91.0, 1}
    };
}

// ── JSON Helpers ──
string je(const string& s) {
    string r;
    for (char c : s) {
        if (c == '"') r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else r += c;
    }
    return r;
}

string qToJson(Question* q, bool ans = true) {
    ostringstream o;
    o << "{\"id\":" << q->id
      << ",\"text\":\"" << je(q->text) << "\""
      << ",\"topic\":\"" << je(q->topic) << "\""
      << ",\"difficulty\":" << q->difficulty
      << ",\"options\":[";
    for (int j = 0; j < (int)q->options.size(); j++) {
        if (j) o << ",";
        o << "\"" << je(q->options[j]) << "\"";
    }
    o << "],\"keywords\":[";
    for (int j = 0; j < (int)q->keywords.size(); j++) {
        if (j) o << ",";
        o << "\"" << je(q->keywords[j]) << "\"";
    }
    o << "]";
    if (ans) o << ",\"answer\":" << q->answer;
    o << "}";
    return o.str();
}

// Minimal JSON value extraction from request body
string jStr(const string& j, const string& k) {
    string s = "\"" + k + "\"";
    size_t p = j.find(s);
    if (p == string::npos) return "";
    p = j.find(':', p + s.size());
    if (p == string::npos) return "";
    p++;
    while (p < j.size() && j[p] == ' ') p++;
    if (p >= j.size() || j[p] != '"') return "";
    p++;
    string r;
    while (p < j.size() && j[p] != '"') {
        if (j[p] == '\\' && p + 1 < j.size()) { r += j[p+1]; p += 2; }
        else { r += j[p]; p++; }
    }
    return r;
}

int jInt(const string& j, const string& k) {
    string s = "\"" + k + "\"";
    size_t p = j.find(s);
    if (p == string::npos) return 0;
    p = j.find(':', p + s.size());
    if (p == string::npos) return 0;
    p++;
    while (p < j.size() && j[p] == ' ') p++;
    try { return stoi(j.substr(p)); } catch (...) { return 0; }
}

bool jBool(const string& j, const string& k) {
    string s = "\"" + k + "\"";
    size_t p = j.find(s);
    if (p == string::npos) return false;
    p = j.find(':', p + s.size());
    if (p == string::npos) return false;
    p++;
    while (p < j.size() && j[p] == ' ') p++;
    return j.substr(p, 4) == "true";
}

vector<int> jIntArr(const string& j, const string& k) {
    vector<int> r;
    string s = "\"" + k + "\"";
    size_t p = j.find(s);
    if (p == string::npos) return r;
    p = j.find('[', p);
    if (p == string::npos) return r;
    size_t e = j.find(']', p);
    if (e == string::npos) return r;
    string sub = j.substr(p + 1, e - p - 1);
    stringstream ss(sub);
    string tok;
    while (getline(ss, tok, ',')) {
        while (!tok.empty() && tok[0] == ' ') tok.erase(0, 1);
        if (!tok.empty()) try { r.push_back(stoi(tok)); } catch (...) {}
    }
    return r;
}

vector<long long> jLongArr(const string& j, const string& k) {
    vector<long long> r;
    string s = "\"" + k + "\"";
    size_t p = j.find(s);
    if (p == string::npos) return r;
    p = j.find('[', p);
    if (p == string::npos) return r;
    size_t e = j.find(']', p);
    if (e == string::npos) return r;
    string sub = j.substr(p + 1, e - p - 1);
    stringstream ss(sub);
    string tok;
    while (getline(ss, tok, ',')) {
        while (!tok.empty() && tok[0] == ' ') tok.erase(0, 1);
        if (!tok.empty()) try { r.push_back(stoll(tok)); } catch (...) {}
    }
    return r;
}

int nextAttempt(int rollNo) {
    int la = 0;
    for (auto& rec : g_records)
        if (rec.rollNo == rollNo) la = max(la, rec.attemptNumber);
    return la + 1;
}

void cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

// ── MAIN ──
int main() {
    seedAll();
    httplib::Server svr;

    // Serve frontend static files
    svr.set_mount_point("/", "./frontend");

    // CORS preflight
    svr.Options("/(.*)", [](const httplib::Request&, httplib::Response& res) {
        cors(res);
        res.status = 204;
    });

    // 1. GET /api/questions — all questions
    svr.Get("/api/questions", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        auto qs = g_bank.getAllQuestions();
        ostringstream o;
        o << "[";
        for (int i = 0; i < (int)qs.size(); i++) {
            if (i) o << ",";
            o << qToJson(qs[i]);
        }
        o << "]";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 2. GET /api/questions/search?keyword=X
    svr.Get("/api/questions/search", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        string kw = req.get_param_value("keyword");
        auto found = g_bank.searchByKeyword(kw);
        ostringstream o;
        o << "[";
        for (int i = 0; i < (int)found.size(); i++) {
            if (i) o << ",";
            o << qToJson(found[i]);
        }
        o << "]";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 3. POST /api/exam/start — DP select questions
    svr.Post("/api/exam/start", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        int level = jInt(req.body, "studentLevel");
        int timeLimit = jInt(req.body, "timeLimit");
        if (level < 1) level = 1; if (level > 5) level = 5;
        if (timeLimit < 60) timeLimit = 60; if (timeLimit > 600) timeLimit = 600;

        AdaptiveExam ae;
        vector<Question*> pool = g_bank.getAllQuestions();
        vector<Question*> sel = ae.selectQuestions(pool, timeLimit, level);

        ostringstream o;
        o << "{\"count\":" << sel.size() << ",\"questions\":[";
        for (int i = 0; i < (int)sel.size(); i++) {
            if (i) o << ",";
            o << qToJson(sel[i], false); // no answer
        }
        o << "]}";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 4. POST /api/exam/submit — submit exam answers
    svr.Post("/api/exam/submit", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        int roll = jInt(req.body, "roll");
        string name = jStr(req.body, "name");
        int examId = jInt(req.body, "examId");
        int level = jInt(req.body, "level");
        auto qIds = jIntArr(req.body, "questionIds");
        auto answers = jIntArr(req.body, "answers");

        int correct = 0;
        int total = (int)qIds.size();
        int correctStreak = 0, wrongStreak = 0;
        bool latestCorrect = false;
        for (int i = 0; i < total; i++) {
            Question* q = g_bank.findById(qIds[i]);
            if (q && i < (int)answers.size() && answers[i] == q->answer) {
                correct++;
                correctStreak++;
                wrongStreak = 0;
                latestCorrect = true;
                if (q) g_topicScoresMap[q->topic].push_back(100.0);
            } else {
                wrongStreak++;
                correctStreak = 0;
                latestCorrect = false;
                if (q) g_topicScoresMap[q->topic].push_back(0.0);
            }
        }

        // Adaptive difficulty adjustment (same as main.cpp)
        AdaptiveExam ae2;
        int nextLevel = ae2.adjustDifficulty(level, latestCorrect, correctStreak, wrongStreak);

        double score = total > 0 ? (100.0 * correct) / total : 0;
        int attempt = nextAttempt(roll);

        if (roll > 0 && !name.empty()) {
            g_resultTree.insert(roll, name, score, examId > 0 ? examId : 501);
            g_records.push_back({roll, name, score, attempt});
            g_submissions.enqueue(roll, examId > 0 ? examId : 501, name, answers, g_nextTs++, "Adaptive Exam");

            // Log activity for proctoring — reset for new exam
            g_activityLog[roll] = ExamActivity();
            g_activityLog[roll].studentName = name;
            g_activityLog[roll].timingAnalyzed = false;

            vector<long long> realTimesMillis = jLongArr(req.body, "answerTimesMillis");
            vector<long long> realTimesSeconds = jLongArr(req.body, "answerTimesSeconds");

            if (!realTimesMillis.empty()) {
                g_activityLog[roll].answerTimesMillis = realTimesMillis;
                g_activityLog[roll].answerTimesSeconds = realTimesSeconds;
            } else {
                // Fallback to mock generation if not provided (e.g. CLI or manual API tests)
                auto now = chrono::steady_clock::now().time_since_epoch();
                long long ms = chrono::duration_cast<chrono::milliseconds>(now).count();
                for (int i = 0; i < total; i++) {
                    g_activityLog[roll].answerTimesMillis.push_back(ms + i * 5000);
                    g_activityLog[roll].answerTimesSeconds.push_back(ms / 1000 + i * 5);
                }
            }

            // Run timing proctoring analysis immediately on submission
            auto& act = g_activityLog[roll];
            if (!act.timingAnalyzed) {
                for (int j = 1; j < (int)act.answerTimesSeconds.size(); j++) {
                    g_proctor.checkTimeGap(roll, act.studentName, act.answerTimesSeconds[j-1], act.answerTimesSeconds[j]);
                }
                g_proctor.checkRapidClicking(roll, act.studentName, act.answerTimesMillis);
                act.timingAnalyzed = true;
            }
        }

        // Build correct answers array
        ostringstream ca;
        ca << "[";
        for (int i = 0; i < total; i++) {
            if (i) ca << ",";
            Question* q = g_bank.findById(qIds[i]);
            ca << (q ? q->answer : -1);
        }
        ca << "]";

        ostringstream o;
        o << "{\"correct\":" << correct
          << ",\"total\":" << total
          << ",\"score\":" << fixed << setprecision(2) << score
          << ",\"attempt\":" << attempt
          << ",\"nextLevel\":" << nextLevel
          << ",\"currentLevel\":" << level
          << ",\"correctAnswers\":" << ca.str() << "}";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 5. GET /api/questions/topic?topic=X — binary search
    svr.Get("/api/questions/topic", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        string topic = req.get_param_value("topic");
        auto pool = g_bank.getAllQuestions();
        auto found = BinarySearch::sortAndSearch(pool, topic);
        ostringstream o;
        o << "[";
        for (int i = 0; i < (int)found.size(); i++) {
            if (i) o << ",";
            o << qToJson(found[i]);
        }
        o << "]";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 6. POST /api/submissions/add
    svr.Post("/api/submissions/add", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        int sid = jInt(req.body, "studentId");
        int eid = jInt(req.body, "examId");
        string name = jStr(req.body, "name");
        auto answers = jIntArr(req.body, "answers");

        if (sid <= 0 || eid <= 0 || name.empty()) {
            cors(res); res.status = 400;
            res.set_content("{\"error\":\"Invalid input\"}", "application/json");
            return;
        }
        g_submissions.enqueue(sid, eid, name, answers, g_nextTs++, "Manual Entry");
        cors(res);
        res.set_content("{\"success\":true}", "application/json");
    });

    // 7. GET /api/submissions
    svr.Get("/api/submissions", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        auto all = g_submissions.getAll();
        ostringstream o;
        o << "[";
        for (int i = 0; i < (int)all.size(); i++) {
            auto* s = all[i];
            if (i) o << ",";
            o << "{\"studentId\":" << s->studentId
              << ",\"examId\":" << s->examId
              << ",\"name\":\"" << je(s->studentName) << "\""
              << ",\"source\":\"" << je(s->source) << "\""
              << ",\"plagiarism\":" << (s->plagiarismFlag ? "true" : "false")
              << ",\"answers\":[";
            for (int j = 0; j < (int)s->answers.size(); j++) {
                if (j) o << ",";
                o << s->answers[j];
            }
            o << "]}";
        }
        o << "]";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 8. POST /api/plagiarism
    svr.Post("/api/plagiarism", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        int flagged = g_submissions.flagPlagiarismByHash();
        auto all = g_submissions.getAll();
        ostringstream o;
        o << "{\"flagged\":" << flagged << ",\"submissions\":[";
        for (int i = 0; i < (int)all.size(); i++) {
            auto* s = all[i];
            if (i) o << ",";

            auto alerts = g_proctor.getAlertsForStudent(s->studentId);
            bool hasTimeGap = false;
            for (auto& a : alerts) {
                if (a.type == ProctorEvent::TIME_GAP) {
                    hasTimeGap = true;
                }
            }

            o << "{\"studentId\":" << s->studentId
              << ",\"name\":\"" << je(s->studentName) << "\""
              << ",\"plagiarism\":" << (s->plagiarismFlag ? "true" : "false")
              << ",\"timeGap\":" << (hasTimeGap ? "true" : "false")
              << ",\"answers\":[";
            for (int j = 0; j < (int)s->answers.size(); j++) {
                if (j) o << ",";
                o << s->answers[j];
            }
            o << "]}";
        }
        o << "]}";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 9. POST /api/review/push
    svr.Post("/api/review/push", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        int qId = jInt(req.body, "questionId");
        int prev = jInt(req.body, "prevAnswer");
        int newA = jInt(req.body, "newAnswer");
        string note = jStr(req.body, "note");
        g_reviewStack.push(qId, prev, newA, note.empty() ? "Web review" : note);
        cors(res);
        res.set_content("{\"success\":true,\"size\":" + to_string(g_reviewStack.getSize()) + "}", "application/json");
    });

    // 10. POST /api/review/pop
    svr.Post("/api/review/pop", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        if (g_reviewStack.isEmpty()) {
            cors(res);
            res.set_content("{\"empty\":true}", "application/json");
            return;
        }
        auto r = g_reviewStack.pop();
        ostringstream o;
        o << "{\"empty\":false,\"questionId\":" << r.questionId
          << ",\"prevAnswer\":" << r.previousAnswer
          << ",\"newAnswer\":" << r.newAnswer
          << ",\"note\":\"" << je(r.note) << "\"}";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 11. POST /api/review/clear
    svr.Post("/api/review/clear", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        g_reviewStack.clear();
        cors(res);
        res.set_content("{\"success\":true}", "application/json");
    });

    // 12. GET /api/review
    svr.Get("/api/review", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        // Access stack contents via peek + pop + re-push
        // Simpler: just expose size and top
        ostringstream o;
        o << "{\"size\":" << g_reviewStack.getSize();
        if (!g_reviewStack.isEmpty()) {
            auto top = g_reviewStack.peek();
            o << ",\"top\":{\"questionId\":" << top.questionId
              << ",\"prevAnswer\":" << top.previousAnswer
              << ",\"newAnswer\":" << top.newAnswer
              << ",\"note\":\"" << je(top.note) << "\"}";
        }
        o << "}";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 12. GET /api/students — BST inorder
    svr.Get("/api/students", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        auto all = g_resultTree.getAllSorted();
        ostringstream o;
        o << "[";
        for (int i = 0; i < (int)all.size(); i++) {
            if (i) o << ",";
            o << "{\"rollNo\":" << all[i]->rollNo
              << ",\"name\":\"" << je(all[i]->name) << "\""
              << ",\"score\":" << fixed << setprecision(2) << all[i]->score
              << ",\"examId\":" << all[i]->examId << "}";
        }
        o << "]";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 13. GET /api/students/search?roll=X
    svr.Get("/api/students/search", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        int roll = 0;
        try { roll = stoi(req.get_param_value("roll")); } catch (...) {}
        BSTNode* n = g_resultTree.search(roll);
        ostringstream o;
        if (!n) {
            o << "{\"found\":false,\"roll\":" << roll << "}";
        } else {
            o << "{\"found\":true,\"rollNo\":" << n->rollNo
              << ",\"name\":\"" << je(n->name) << "\""
              << ",\"score\":" << fixed << setprecision(2) << n->score
              << ",\"examId\":" << n->examId << "}";
        }
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 14. GET /api/ranking — merge sort
    svr.Get("/api/ranking", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        auto ranked = MergeSort::dedup(g_records);
        ostringstream o;
        o << "[";
        for (int i = 0; i < (int)ranked.size(); i++) {
            if (i) o << ",";
            o << "{\"rank\":" << (i + 1)
              << ",\"rollNo\":" << ranked[i].rollNo
              << ",\"name\":\"" << je(ranked[i].name) << "\""
              << ",\"score\":" << fixed << setprecision(2) << ranked[i].score << "}";
        }
        o << "]";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 15. POST /api/proctoring — analyze
    svr.Post("/api/proctoring", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        int sid = jInt(req.body, "roll");
        bool cp = jBool(req.body, "copyPaste");
        bool ts = jBool(req.body, "tabSwitch");
        bool fa = jBool(req.body, "faceAbsent");

        if (!g_activityLog.count(sid)) {
            cors(res);
            res.set_content("{\"error\":\"No exam activity for this roll number\"}", "application/json");
            return;
        }

        auto& act = g_activityLog[sid];
        if (!act.timingAnalyzed) {
            for (int i = 1; i < (int)act.answerTimesSeconds.size(); i++)
                g_proctor.checkTimeGap(sid, act.studentName, act.answerTimesSeconds[i-1], act.answerTimesSeconds[i]);
            g_proctor.checkRapidClicking(sid, act.studentName, act.answerTimesMillis);
            act.timingAnalyzed = true;
        }
        if (cp) g_proctor.reportCopyPaste(sid, act.studentName, act.answerTimesMillis.back());
        if (ts) g_proctor.reportTabSwitch(sid, act.studentName, act.answerTimesMillis.back());
        if (fa) g_proctor.reportFaceAbsent(sid, act.studentName, act.answerTimesMillis.back());

        auto alerts = g_proctor.getAlertsForStudent(sid);
        bool suspicious = g_proctor.isSuspicious(sid);

        // Build timing array (same as terminal "Recorded Exam Timing")
        ostringstream o;
        o << "{\"name\":\"" << je(act.studentName) << "\""
          << ",\"roll\":" << sid
          << ",\"alertCount\":" << alerts.size()
          << ",\"suspicious\":" << (suspicious ? "true" : "false")
          << ",\"timing\":[";
        for (int i = 0; i < (int)act.answerTimesSeconds.size(); i++) {
            if (i) o << ",";
            o << "{\"answer\":" << (i + 1)
              << ",\"seconds\":" << act.answerTimesSeconds[i]
              << ",\"millis\":" << act.answerTimesMillis[i] << "}";
        }
        o << "],\"timingAnalyzed\":" << (act.timingAnalyzed ? "true" : "false")
          << ",\"alerts\":[";
        for (int i = 0; i < (int)alerts.size(); i++) {
            if (i) o << ",";
            o << "{\"type\":\"" << je(eventName(alerts[i].type)) << "\""
              << ",\"detail\":\"" << je(alerts[i].detail) << "\"}";
        }
        o << "]}";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    // 16. GET /api/analytics
    svr.Get("/api/analytics", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(g_mtx);
        vector<double> scores;
        for (auto* n : g_resultTree.getAllSorted())
            scores.push_back(n->score);
        auto stats = PerformanceAnalytics::compute(scores);
        // Genuine topic-wise averages from C++ PerformanceAnalytics
        auto topicAvg = PerformanceAnalytics::topicWiseAverage(g_topicScoresMap);
        ostringstream o;
        o << fixed << setprecision(2)
          << "{\"average\":" << stats.average
          << ",\"highest\":" << stats.highest
          << ",\"lowest\":" << stats.lowest
          << ",\"stdDev\":" << stats.stdDev
          << ",\"count\":" << scores.size()
          << ",\"topicWise\":[";
        int ti = 0;
        for (auto& entry : topicAvg) {
            if (ti++) o << ",";
            o << "{\"topic\":\"" << je(entry.first) << "\",\"average\":" << entry.second << "}";
        }
        o << "]}";
        cors(res);
        res.set_content(o.str(), "application/json");
    });

    cout << "\n============================================================\n";
    cout << " Adaptive Exam Platform — HTTP Server\n";
    cout << " Open http://localhost:8080 in your browser\n";
    cout << "============================================================\n";

    svr.listen("0.0.0.0", 8080);
    return 0;
}
