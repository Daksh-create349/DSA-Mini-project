// ============================================================
// main.cpp
// Interactive demo: Adaptive Online Examination Platform
// ============================================================
#include <chrono>
#include <limits>
#include <unordered_map>
#include "question_bank.cpp"
#include "submission_queue_stack.cpp"
#include "adaptive_dp_sort_search.cpp"
#include "analytics_bst_proctor.cpp"

struct ExamActivity {
    string studentName;
    vector<long long> answerTimesSeconds;
    vector<long long> answerTimesMillis;
    bool timingAnalyzed = false;
};

int readInt(const string& prompt, int minValue, int maxValue) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minValue && value <= maxValue) {
            return value;
        }

        cout << "Invalid input. Enter a number between "
             << minValue << " and " << maxValue << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

char readYesNo(const string& prompt) {
    char value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value == 'y' || value == 'Y' || value == 'n' || value == 'N')
                return value;
        }

        cout << "Invalid input. Enter y or n.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

long long readLongLong(const string& prompt, long long minValue, long long maxValue) {
    long long value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minValue && value <= maxValue) {
            return value;
        }

        cout << "Invalid input. Enter a number between "
             << minValue << " and " << maxValue << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void seedQuestions(QuestionBank& bank) {
    bank.addQuestion("What is the time complexity of binary search?", "Searching", 2,
                     {"O(n)", "O(log n)", "O(n log n)", "O(1)"}, 1,
                     {"binary", "search", "complexity"});
    bank.addQuestion("Which data structure follows FIFO?", "Queue", 1,
                     {"Stack", "Queue", "Tree", "Graph"}, 1,
                     {"fifo", "queue"});
    bank.addQuestion("Which traversal of BST gives sorted order?", "BST", 3,
                     {"Preorder", "Postorder", "Inorder", "Level order"}, 2,
                     {"bst", "tree", "inorder"});
    bank.addQuestion("Which algorithm is stable and divide-and-conquer?", "Sorting", 3,
                     {"Quick Sort", "Merge Sort", "Selection Sort", "Heap Sort"}, 1,
                     {"merge", "sort", "stable"});
    bank.addQuestion("Dynamic programming avoids repeated work using what?", "DP", 4,
                     {"Recursion only", "Memoization", "Randomization", "Hash collision"}, 1,
                     {"dp", "memoization", "optimal"});
    bank.addQuestion("A stack removes the most recently inserted item using which rule?", "Stack", 1,
                     {"FIFO", "LIFO", "Priority", "Hashing"}, 1,
                     {"stack", "lifo"});
}

void seedStudents(StudentBST& resultTree, vector<StudentRecord>& records) {
    resultTree.insert(101, "Aarav", 86.0, 501);
    resultTree.insert(102, "Meera", 82.5, 501);
    resultTree.insert(103, "Kabir", 91.0, 501);

    records = {
        {101, "Aarav", 86.0, 1},
        {102, "Meera", 79.0, 1},
        {102, "Meera", 82.5, 2},
        {103, "Kabir", 91.0, 1}
    };
}

int nextAttemptNumber(const vector<StudentRecord>& records, int rollNo) {
    int latestAttempt = 0;
    for (const auto& record : records)
        if (record.rollNo == rollNo)
            latestAttempt = max(latestAttempt, record.attemptNumber);
    return latestAttempt + 1;
}

void showMenu() {
    cout << "\n================ MENU ================\n";
    cout << "1. Display question bank\n";
    cout << "2. Search questions by keyword prefix\n";
    cout << "3. Take adaptive exam using DP\n";
    cout << "4. Binary search questions by topic\n";
    cout << "5. Add a submission to queue\n";
    cout << "6. Display submission queue\n";
    cout << "7. Run plagiarism check\n";
    cout << "8. Use answer review stack\n";
    cout << "9. Display student BST and search roll number\n";
    cout << "10. Show ranking using merge sort\n";
    cout << "11. Analyze proctoring from exam activity\n";
    cout << "12. Show performance analytics from student scores\n";
    cout << "0. Exit\n";
    cout << "Enter choice: ";
}

int main() {
    QuestionBank bank;
    SubmissionQueue submissions;
    AnswerReviewStack reviewStack;
    StudentBST resultTree;
    vector<StudentRecord> records;
    unordered_map<int, ExamActivity> activityLog;
    AutoProctor proctor;
    long long nextTimestamp = 1000;

    // Track genuine per-topic performance across all exams
    // Key: topic name, Value: vector of per-question scores (100 if correct, 0 if wrong)
    unordered_map<string, vector<double>> topicScoresMap;

    seedQuestions(bank);
    seedStudents(resultTree, records);

    cout << "============================================================\n";
    cout << " Adaptive Online Examination Platform with AI Proctoring\n";
    cout << "============================================================\n";
    cout << "Sample data loaded. Use the menu to interact with the system.\n";

    int choice = -1;
    do {
        showMenu();
        if (!(cin >> choice)) {
            cout << "Invalid choice. Enter a number from the menu.\n";
            choice = -1;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: {
                bank.displayAll();
                break;
            }

            case 2: {
                string prefix;
                cout << "Enter keyword prefix: ";
                cin >> prefix;
                auto found = bank.searchByKeyword(prefix);
                cout << "\n===== Search Results =====\n";
                if (found.empty()) cout << "  No questions found.\n";
                for (Question* q : found)
                    cout << "  Q" << q->id << " | " << q->topic << " | " << q->text << "\n";
                break;
            }

            case 3: {
                int roll = readInt("Student roll number: ", 1, 999999);
                int examId = readInt("Exam id: ", 1, 999999);
                string name;
                cout << "Student name: ";
                cin >> name;
                int level = readInt("Enter student level (1-5): ", 1, 5);
                int timeLimit = readInt("Enter time limit in seconds (60-600): ", 60, 600);

                AdaptiveExam adaptiveExam;
                vector<Question*> pool = bank.getAllQuestions();
                vector<Question*> selected = adaptiveExam.selectQuestions(pool, timeLimit, level);
                if (selected.empty()) {
                    cout << "  No suitable questions selected.\n";
                } else {
                    vector<int> answers;
                    int correctAnswers = 0;
                    int correctStreak = 0;
                    int wrongStreak = 0;
                    bool latestCorrect = false;
                    auto examStart = chrono::steady_clock::now();

                    cout << "\n===== Adaptive Exam Started =====\n";
                    for (Question* q : selected)
                    {
                        cout << "\nQ" << q->id << " [" << q->topic << "] diff=" << q->difficulty
                             << "\n" << q->text << "\n";
                        for (int i = 0; i < (int)q->options.size(); i++)
                            cout << "  " << i << ". " << q->options[i] << "\n";

                        int answer = readInt("Your answer (0-3): ", 0, 3);
                        answers.push_back(answer);

                        auto now = chrono::steady_clock::now();
                        long long elapsedMs = chrono::duration_cast<chrono::milliseconds>(now - examStart).count();
                        long long elapsedSec = chrono::duration_cast<chrono::seconds>(now - examStart).count();
                        activityLog[roll].studentName = name;
                        activityLog[roll].answerTimesMillis.push_back(elapsedMs);
                        activityLog[roll].answerTimesSeconds.push_back(elapsedSec);

                        if (answer == q->answer) {
                            cout << "  Correct.\n";
                            correctAnswers++;
                            correctStreak++;
                            wrongStreak = 0;
                            latestCorrect = true;
                            topicScoresMap[q->topic].push_back(100.0);
                        } else {
                            cout << "  Wrong. Correct answer was option " << q->answer << ".\n";
                            wrongStreak++;
                            correctStreak = 0;
                            latestCorrect = false;
                            topicScoresMap[q->topic].push_back(0.0);
                        }
                    }

                    double score = (100.0 * correctAnswers) / selected.size();
                    int attempt = nextAttemptNumber(records, roll);
                    resultTree.insert(roll, name, score, examId);
                    records.push_back({roll, name, score, attempt});
                    submissions.enqueue(roll, examId, name, answers, nextTimestamp++, "Adaptive Exam");

                    cout << "\n===== Exam Submitted =====\n";
                    cout << "  Student : " << name << " (Roll:" << roll << ")\n";
                    cout << "  Correct : " << correctAnswers << "/" << selected.size() << "\n";
                    cout << "  Score   : " << fixed << setprecision(2) << score << "\n";
                    cout << "  Attempt : " << attempt << "\n";
                    cout << "  Saved to submission queue, BST results, ranking records, analytics data,\n";
                    cout << "  and proctoring activity log.\n";

                    int nextLevel = adaptiveExam.adjustDifficulty(level, latestCorrect,
                                                                  correctStreak, wrongStreak);
                    cout << "  Next difficulty level: " << nextLevel << "\n";
                }
                break;
            }

            case 4: {
                string topic;
                cout << "Enter exact topic, e.g. Stack, DP, BST, Sorting, Searching: ";
                cin >> topic;
                vector<Question*> pool = bank.getAllQuestions();
                auto found = BinarySearch::sortAndSearch(pool, topic);
                cout << "\n===== Binary Search Results =====\n";
                if (found.empty()) cout << "  No questions found for this topic.\n";
                for (Question* q : found)
                    cout << "  Q" << q->id << " | " << q->text << "\n";
                break;
            }

            case 5: {
                int sid, eid, answerCount;
                string name;
                vector<int> answers;
                sid = readInt("Student id: ", 1, 999999);
                eid = readInt("Exam id: ", 1, 999999);
                cout << "Student name: ";
                cin >> name;
                answerCount = readInt("Number of answers (1-50): ", 1, 50);
                cout << "Enter answers as option indexes: ";
                for (int i = 0; i < answerCount; i++) {
                    int ans = readInt("", 0, 3);
                    answers.push_back(ans);
                }
                submissions.enqueue(sid, eid, name, answers, nextTimestamp++, "Manual Entry");
                break;
            }

            case 6: {
                submissions.display();
                break;
            }

            case 7: {
                submissions.flagPlagiarismByHash();
                submissions.display();
                break;
            }

            case 8: {
                int qId = readInt("Question id: ", 1, 999999);
                int oldAns = readInt("Previous answer (0-3): ", 0, 3);
                int newAns = readInt("New answer (0-3): ", 0, 3);
                reviewStack.push(qId, oldAns, newAns, "Manual review update");
                reviewStack.displayHistory();

                char undo = readYesNo("Undo latest review? (y/n): ");
                if (undo == 'y' || undo == 'Y') reviewStack.pop();
                reviewStack.displayHistory();
                break;
            }

            case 9: {
                resultTree.displayAll();
                int roll = readInt("Enter roll number to search: ", 1, 999999);
                resultTree.displayResult(roll);
                break;
            }

            case 10: {
                vector<StudentRecord> latestAttempts = MergeSort::dedup(records);
                MergeSort::display(latestAttempts);
                break;
            }

            case 11: {
                int sid = readInt("Student roll number for proctoring report: ", 1, 999999);
                if (!activityLog.count(sid)) {
                    cout << "  No recorded exam activity for this roll number.\n";
                    cout << "  Ask the student to take the exam using option 3 first.\n";
                    break;
                }

                ExamActivity& activity = activityLog[sid];
                cout << "\n===== Recorded Exam Timing =====\n";
                for (int i = 0; i < (int)activity.answerTimesSeconds.size(); i++) {
                    cout << "  Answer " << (i + 1)
                         << " submitted at " << activity.answerTimesSeconds[i]
                         << "s (" << activity.answerTimesMillis[i] << "ms)\n";
                }

                if (!activity.timingAnalyzed) {
                    for (int i = 1; i < (int)activity.answerTimesSeconds.size(); i++) {
                        proctor.checkTimeGap(sid, activity.studentName,
                                             activity.answerTimesSeconds[i - 1],
                                             activity.answerTimesSeconds[i]);
                    }
                    proctor.checkRapidClicking(sid, activity.studentName, activity.answerTimesMillis);
                    activity.timingAnalyzed = true;
                    cout << "  Timing analysis completed automatically.\n";
                } else {
                    cout << "  Timing was already analyzed earlier, so duplicate timing alerts were skipped.\n";
                }

                char copyPaste = readYesNo("Was copy-paste detected externally? (y/n): ");
                if (copyPaste == 'y' || copyPaste == 'Y')
                    proctor.reportCopyPaste(sid, activity.studentName, activity.answerTimesMillis.back());

                char tabSwitch = readYesNo("Was tab switch detected externally? (y/n): ");
                if (tabSwitch == 'y' || tabSwitch == 'Y')
                    proctor.reportTabSwitch(sid, activity.studentName, activity.answerTimesMillis.back());

                char faceAbsent = readYesNo("Was face absent detected externally? (y/n): ");
                if (faceAbsent == 'y' || faceAbsent == 'Y')
                    proctor.reportFaceAbsent(sid, activity.studentName, activity.answerTimesMillis.back());

                proctor.displayAllAlerts();
                proctor.generateReport(sid, activity.studentName);
                break;
            }

            case 12: {
                vector<double> scores;
                for (BSTNode* node : resultTree.getAllSorted())
                    scores.push_back(node->score);

                // Use genuine per-topic scores tracked during exams
                PerformanceStats stats = PerformanceAnalytics::compute(scores);
                stats.topicWiseAvg = PerformanceAnalytics::topicWiseAverage(topicScoresMap);
                PerformanceAnalytics::displayStats(stats, "Exam 501");
                break;
            }

            case 0:
                cout << "Exiting project demo.\n";
                break;

            default:
                cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 0);

    while (!submissions.isEmpty()) {
        Submission* processed = submissions.dequeue();
        delete processed;
    }

    return 0;
}
