# PS_04: Adaptive Online Examination Platform with AI Proctoring

## GROUP 4 

- Daksh Srivastava
- Sumit Shingole
- Ayush Yadgiri
- Anurag Kharke

## Project Summary

This is a C++ Data Structures and Algorithms mini project based on an adaptive online examination platform. The system stores questions, searches questions by keyword, generates an adaptive exam, records submissions, checks plagiarism, stores answer review history, ranks students, searches student results using a BST, performs proctoring analysis, and calculates performance analytics.

The project is menu-driven. It is not just a static demo. A student can take an adaptive exam from the menu, submit answers, get a score, and then that result becomes available in the BST, ranking system, submission queue, and analytics module.

## Problem Statement

Build an educational platform with:

- Adaptive difficulty where questions become harder or easier based on student performance.
- Auto-proctoring that flags suspicious patterns.
- Plagiarism detection using hash comparison.
- Performance analytics for student scores.
- Dynamic programming for optimal question selection.
- Trie-based keyword search for fast question lookup.

## Implementation Checklist

### Core Features

- [x] Question bank using linked list metadata.
- [x] Linear search by topic and difficulty.
- [x] Trie keyword search for fast prefix-based question lookup.
- [x] Adaptive exam using dynamic programming.
- [x] Question selection based on topic coverage, difficulty, and time limit.
- [x] Auto-proctoring for time gap detection.
- [x] Auto-proctoring for rapid clicking detection.
- [x] External proctoring reports for copy-paste, tab switch, and face absent events.
- [x] Stack for answer review history.
- [x] Stack-based undo of latest answer review.
- [x] Queue for submission processing.
- [x] Hash-based plagiarism check for duplicate answer patterns.
- [x] Merge sort for student ranking.
- [x] Deduplication by latest attempt number.
- [x] Binary search on topic-sorted questions.
- [x] BST for student result storage and roll number search.
- [x] Performance analytics: average, highest, lowest, standard deviation.
- [x] Topic-wise analytics using hash map.
- [x] Menu-driven interactive program.

### Extra Custom Features

- [x] Students can take an actual adaptive exam from the terminal.
- [x] Exam answers are evaluated immediately.
- [x] Student score is saved into the BST.
- [x] Student score is saved into ranking records.
- [x] Student submission is saved into the submission queue.
- [x] Exam answer timings are recorded automatically.
- [x] Proctoring option analyzes recorded exam activity automatically.
- [x] Input validation prevents invalid menu choices and invalid answer options.
- [x] README includes usage guide, feature mapping, complexity analysis, and project explanation.

## Data Structures And Algorithms Used

| Data Structure / Algorithm | Used In | Purpose |
| --- | --- | --- |
| Linked List | Question bank | Store questions one after another using `next` pointer |
| Linear Search | Question bank | Search questions by topic, difficulty, or id |
| Trie | Keyword search | Fast prefix search for question keywords |
| Dynamic Programming | Adaptive exam | Select best questions within time limit |
| Queue | Submission handling | Process submissions in FIFO order |
| Stack | Answer review | Store review history and undo latest review |
| Hash Map | Plagiarism and analytics | Detect duplicate answer patterns and store topic-wise marks |
| Merge Sort | Ranking | Sort students by score |
| Binary Search | Topic search | Search questions after sorting by topic |
| BST | Student records | Store and search results by roll number |
| Vector | Many modules | Dynamic storage of questions, answers, scores, alerts |

## File Structure

```text
DSA Mini project/
|
|-- main.cpp
|-- question_bank.cpp
|-- submission_queue_stack.cpp
|-- adaptive_dp_sort_search.cpp
|-- analytics_bst_proctor.cpp
|-- README.md
```

## File Wise Explanation

### 1. `main.cpp`

This is the main controller file. It connects all modules and provides the interactive menu.

Main responsibilities:

- Loads sample questions.
- Loads sample student result records.
- Displays menu options.
- Takes user input.
- Calls the correct class and function based on menu choice.
- Allows a student to take an adaptive exam.
- Saves exam result into BST and ranking records.
- Saves exam submission into queue.
- Records answer timestamps for proctoring.
- Calls analytics and proctoring reports.

Important helper functions:

| Function | Purpose |
| --- | --- |
| `readInt()` | Reads validated integer input |
| `readYesNo()` | Reads only `y` or `n` |
| `readLongLong()` | Reads large validated numeric input |
| `seedQuestions()` | Adds sample questions to question bank |
| `seedStudents()` | Adds sample student records |
| `nextAttemptNumber()` | Finds next attempt number for a roll number |
| `showMenu()` | Prints the main menu |

### 2. `question_bank.cpp`

This file handles the question bank.

Main classes and structures:

- `Question`
- `TrieNode`
- `Trie`
- `QuestionBank`

Each question contains:

- question id
- question text
- topic
- difficulty
- four options
- correct answer index
- keywords
- pointer to next question

The `QuestionBank` stores questions using a linked list. This satisfies the linked list requirement.

The `Trie` stores keywords and topics. This allows fast prefix searching.

Example:

If the keywords are:

```text
stack
sorting
search
```

and the user searches:

```text
st
```

the trie can return matching question ids for `stack` and `sorting`.

### 3. `submission_queue_stack.cpp`

This file has two main parts:

- Submission queue
- Answer review stack

The `SubmissionQueue` uses FIFO behavior:

```text
First In, First Out
```

If students submit in this order:

```text
Student A -> Student B -> Student C
```

then evaluation can happen in the same order:

```text
Student A -> Student B -> Student C
```

The `AnswerReviewStack` uses LIFO behavior:

```text
Last In, First Out
```

This is useful for undo operations. The latest review is removed first.

Plagiarism check:

The plagiarism function converts answers into a hash key.

Example:

```text
Answers: 1 1 2 1
Hash key: 1#1#2#1#
```

If another student has the same answer pattern, the second submission gets flagged.

### 4. `adaptive_dp_sort_search.cpp`

This file contains:

- Adaptive exam question selection using dynamic programming.
- Merge sort for ranking.
- Deduplication by latest attempt.
- Binary search for topic-wise question lookup.

#### Dynamic Programming

The adaptive exam works like a knapsack problem.

Simple idea:

```text
Time is limited.
Questions have value.
Questions have difficulty.
The program selects the best possible set of questions within the time limit.
```

The program first filters questions based on student level. If the student level is 3, it selects questions near that level, such as difficulty 2, 3, or 4.

Then dynamic programming chooses the best combination.

#### Merge Sort

Merge sort is used for ranking students by score. Higher score comes first.

Example:

```text
1. Riya   100
2. Kabir  91
3. Aarav  86
4. Meera  82.5
```

The `dedup()` function keeps only the latest attempt for each roll number.

#### Binary Search

Binary search is used for topic search after sorting questions by topic.

Example:

```text
Search topic: Stack
```

The program sorts questions by topic, then applies binary search.

### 5. `analytics_bst_proctor.cpp`

This file contains:

- Student BST
- Auto-proctoring engine
- Performance analytics

#### Student BST

Student results are stored by roll number using a Binary Search Tree.

Example:

```text
        102
       /   \
    101     103
```

Searching roll number 103 is faster than checking every student one by one in a large data set.

#### Auto-Proctoring

The proctoring engine can detect:

- large time gaps between answers
- rapid clicking patterns
- copy-paste events
- tab switch events
- face absent events

Time gap and rapid clicking are calculated from recorded exam activity.

Copy-paste, tab switch, and face absent are entered as external observations because a normal terminal-based C++ program cannot access browser activity or webcam data.

#### Performance Analytics

The analytics module calculates:

- average score
- highest score
- lowest score
- standard deviation
- topic-wise average

## Menu Guide

### Option 1: Display Question Bank

```text
1. Display question bank
```

Shows all questions stored in the linked list.

Displays:

- question id
- topic
- difficulty
- question text

DSA shown:

- Linked List
- Linear Traversal

### Option 2: Search Questions By Keyword Prefix

```text
2. Search questions by keyword prefix
```

Example input:

```text
st
```

Can find questions with keywords like:

```text
stack
sorting
```

DSA shown:

- Trie
- Prefix Search

### Option 3: Take Adaptive Exam Using DP

```text
3. Take adaptive exam using DP
```

This is one of the most important options.

It asks for:

- student roll number
- exam id
- student name
- student level
- time limit

Then it selects questions using dynamic programming.

The student answers questions. The program then:

- checks correct and wrong answers
- calculates score
- saves result into BST
- saves ranking record
- saves submission into queue
- records answer timings for proctoring
- calculates next difficulty level

DSA shown:

- Dynamic Programming
- Queue
- BST
- Merge Sort records
- Vector

### Option 4: Binary Search Questions By Topic

```text
4. Binary search questions by topic
```

Example input:

```text
Stack
```

The program sorts questions by topic and then applies binary search.

DSA shown:

- Sorting
- Binary Search

### Option 5: Add A Submission To Queue

```text
5. Add a submission to queue
```

This manually adds a student submission.
This is kept just for Demo for option 3 (JUST TO EXPLAIN HOW IT WORKS.)

Example:

```text
Student id: 201
Exam id: 501
Student name: Riya
Number of answers: 4
Answers: 1 1 2 1
```

DSA shown:

- Queue

### Option 6: Display Submission Queue

```text
6. Display submission queue
```

Displays all pending submissions in FIFO order.

DSA shown:

- Queue Traversal

### Option 7: Run Plagiarism Check

```text
7. Run plagiarism check
```

Checks whether two submissions have the same answer pattern.

Example:

```text
Student A: 1 1 2 1
Student B: 1 1 2 1
```

The answer pattern is converted into a hash key:

```text
1#1#2#1#
```

If the same key appears again, the later submission is flagged.

DSA shown:

- Hash Map
- String Key Generation

### Option 8: Use Answer Review Stack

```text
8. Use answer review stack
```

Stores answer corrections.

Example:

```text
Question id: 4
Previous answer: 0
New answer: 1
Undo latest review? y
```

Undo removes the latest review first.

DSA shown:

- Stack
- LIFO

### Option 9: Display Student BST And Search Roll Number

```text
9. Display student BST and search roll number
```

Displays all students in sorted roll number order.

Then it asks for a roll number to search.

Example:

```text
Enter roll number: 201
```

If student 201 gave the exam through option 3, they will appear here.

DSA shown:

- Binary Search Tree
- Inorder Traversal
- BST Search

### Option 10: Show Ranking Using Merge Sort

```text
10. Show ranking using merge sort
```

Displays students ranked by score.

Also keeps only the latest attempt for duplicate roll numbers.

DSA shown:

- Merge Sort
- Hash Map Deduplication

### Option 11: Analyze Proctoring From Exam Activity

```text
11. Analyze proctoring from exam activity
```

This option uses recorded exam timings from option 3.

It automatically checks:

- time gap between answers
- rapid clicking timestamps

It also asks whether external suspicious events were detected:

- copy-paste
- tab switch
- face absent

Those external events are asked manually because terminal C++ cannot monitor browser tabs or webcam.

DSA shown:

- Vector Traversal
- Rule-Based Detection

### Option 12: Show Performance Analytics From Student Scores

```text
12. Show performance analytics from student scores
```

Calculates:

- average
- highest
- lowest
- standard deviation
- optional topic-wise average

Scores come from the BST result records.

DSA shown:

- Vector
- Hash Map
- Traversal

### Option 0: Exit

```text
0. Exit
```

Closes the program safely.

## Recommended Presentation Flow

Use this order during demo:

```text
1  Display question bank
2  Search prefix st
3  Take adaptive exam
9  Search the new student roll number
10 Show ranking
6  Display submission queue
7  Run plagiarism check
8  Use answer review stack
11 Analyze proctoring from recorded activity
12 Show performance analytics
0  Exit
```

This flow shows every important data structure clearly.

## Sample Run Inputs

### Take Adaptive Exam

Choose:

```text
3
```

Example input:

```text
Student roll number: 201
Exam id: 501
Student name: Riya
Student level: 3
Time limit: 180
```

Then answer the displayed questions using option indexes:

```text
0, 1, 2, or 3
```

After submission, use option 9 and search:

```text
201
```

The new student will appear in the BST.

## Compile And Run

### Recommended Command

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o exam_platform
./exam_platform
```

### Alternative Command

```bash
g++ -std=c++17 -Wall -Wextra -pedantic *.cpp -o exam_platform
./exam_platform
```

## Complexity Analysis

The following symbols are used:

| Symbol | Meaning |
| --- | --- |
| `Q` | Number of questions |
| `K` | Average keyword length |
| `P` | Prefix length |
| `M` | Number of matched questions |
| `S` | Number of submissions |
| `A` | Number of answers in one submission |
| `R` | Number of student records |
| `N` | Number of eligible questions for adaptive exam |
| `W` | Time limit used as DP capacity |
| `H` | Height of BST |
| `E` | Number of proctoring events or recorded answer timings |
| `T` | Number of topics used in analytics |

### Question Bank

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Add question to linked list | `O(Q)` | `O(1)` extra | Current implementation appends by walking to the end of the list |
| Store question data | `O(1)` | `O(1)` per question | Stores id, text, topic, difficulty, options, answer, keywords |
| Search by topic | `O(Q)` | `O(M)` | Linear scan through linked list |
| Search by difficulty | `O(Q)` | `O(M)` | Linear scan through linked list |
| Find by id | `O(Q)` | `O(1)` | Linear scan through linked list |
| Display all questions | `O(Q)` | `O(1)` extra | Traverses the linked list |

### Trie Keyword Search

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Insert keyword | `O(K)` | `O(K)` worst case | Creates trie nodes for keyword characters |
| Search prefix | `O(P + M)` | `O(M)` | Traverses prefix, then collects matching question ids |
| Full trie storage | `O(total characters)` | `O(total characters)` | Depends on total keyword and topic characters |

### Adaptive Exam Using Dynamic Programming

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Filter eligible questions | `O(Q)` | `O(N)` | Keeps questions near student difficulty level |
| Count topic frequency | `O(N)` average | `O(T)` | Uses hash map |
| DP table fill | `O(N * W)` | `O(N * W)` | Standard 0/1 knapsack style DP |
| Backtrack selected questions | `O(N)` | `O(N)` | Finds selected questions from DP table |
| Evaluate answers | `O(N)` | `O(N)` | Stores answers and checks correctness |

### Submission Queue

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Enqueue submission | `O(1)` | `O(A)` | Rear pointer allows constant insertion |
| Dequeue submission | `O(1)` | `O(1)` | Removes front node |
| Display queue | `O(S * A)` | `O(1)` extra | Prints all submissions and their answers |
| Get all submissions | `O(S)` | `O(S)` | Returns submission pointers in a vector |

### Plagiarism Check

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Create answer hash key | `O(A)` | `O(A)` | Converts answers to string key |
| Check all submissions | `O(S * A)` average | `O(S * A)` | Stores answer patterns in hash map |

### Answer Review Stack

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Push review | `O(1)` amortized | `O(1)` per review | Adds to vector end |
| Pop review | `O(1)` | `O(1)` | Removes latest review |
| Peek review | `O(1)` | `O(1)` | Reads latest review |
| Display history | `O(R)` | `O(1)` extra | Traverses review records |

Here `R` means number of review records for this table.

### Merge Sort Ranking

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Merge sort | `O(R log R)` | `O(R)` | Sorts students by descending score |
| Deduplicate attempts | `O(R)` average | `O(R)` | Hash map keeps latest attempt |
| Display ranking | `O(R)` | `O(1)` extra | Prints ranked records |

Here `R` means number of student records.

### Binary Search On Topic-Sorted Questions

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Sort questions by topic | `O(Q log Q)` | `O(Q)` | Sorts copied vector of question pointers |
| Binary search topic | `O(log Q)` | `O(1)` | Finds one matching topic index |
| Expand matching range | `O(M)` | `O(M)` | Collects all questions with same topic |
| Total sort and search | `O(Q log Q + M)` | `O(Q + M)` | Includes sorting copy and result vector |

### Student BST

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Insert result | `O(H)` | `O(H)` recursion | `H` is height of BST |
| Search by roll number | `O(H)` | `O(H)` recursion | Faster if tree is balanced |
| Inorder display | `O(R)` | `O(R)` result vector | Displays sorted by roll number |
| Range query | `O(H + M)` | `O(M)` | Finds roll numbers in range |

Worst case BST height is `O(R)` if inserted in sorted order. Average case height is closer to `O(log R)`.

### Proctoring

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Record answer timing | `O(1)` per answer | `O(E)` total | Stores answer timestamp during exam |
| Check time gaps | `O(E)` | `O(1)` extra | Compares adjacent timestamps |
| Check rapid clicking | `O(E)` | `O(1)` extra | Counts very small time differences |
| Store alert | `O(1)` amortized | `O(1)` per alert | Pushes alert into vector |
| Generate student report | `O(number of alerts)` | `O(number of matching alerts)` | Filters alerts for one student |

### Performance Analytics

| Operation | Time Complexity | Space Complexity | Explanation |
| --- | --- | --- | --- |
| Average, highest, lowest | `O(R)` | `O(1)` | Traverses scores |
| Standard deviation | `O(R)` | `O(1)` | Second pass over scores |
| Topic-wise average | `O(total topic marks)` | `O(T)` | Uses hash map |

## Why Each DSA Is Suitable

### Linked List For Question Bank

The question bank needs dynamic insertion. A linked list allows new questions to be added without fixed array size.

### Trie For Keyword Search

A trie is suitable when searching by prefix. If the user types `st`, the trie can quickly find matching keywords such as `stack` or `sorting`.

### Dynamic Programming For Adaptive Exam

The exam has a time limit. Each question has a value. The goal is to select the best set of questions within the available time. This is similar to the 0/1 knapsack problem, so dynamic programming is suitable.

### Queue For Submissions

Submissions should be processed in the order they arrive. A queue follows FIFO, so it matches this requirement naturally.

### Stack For Answer Review

Answer review needs undo. Undo always removes the most recent change first. A stack follows LIFO, so it is the correct structure.

### Hashing For Plagiarism

Answer patterns can be converted into strings and stored in a hash map. Repeated answer patterns can be detected efficiently.

### Merge Sort For Ranking

Merge sort is stable and has predictable `O(n log n)` time complexity. It is suitable for ranking students by score.

### Binary Search For Topic Lookup

After sorting questions by topic, binary search can find the topic faster than linear search.

### BST For Student Results

Student results are searched by roll number. BST supports efficient insert and search when reasonably balanced.

## Important Notes

- The project is terminal based.
- It does not use a database.
- Data exists only while the program is running.
- Sample data is loaded at startup for easy testing.
- New exam results entered through option 3 are saved during the same runtime.
- Copy-paste, tab switch, and face absent are external observations in this terminal version.
- Time gap and rapid clicking are calculated from recorded answer timings.

## Limitations

- Student names currently accept one word only, such as `Riya`, not `Riya Sharma`.
- Topic search is exact and case-sensitive, so use `Stack`, not `stack`.
- BST is not self-balancing, so worst-case search can become `O(R)`.
- Data is not stored permanently after program exit.
- Terminal C++ cannot directly detect browser tab switch or webcam face absence.

## Possible Future Improvements

- Add file handling to save questions, submissions, and results permanently.
- Add login system for students and admins.
- Add case-insensitive topic search.
- Add AVL tree or Red-Black tree instead of normal BST.
- Add real browser event tracking in a web version.
- Add webcam-based face detection in a GUI or web version.
- Add admin option to add questions from menu.
- Add CSV export for reports.

## One Paragraph Project Explanation

This project is an adaptive online examination platform implemented in C++. It uses a linked list and trie for question storage and keyword search, dynamic programming for adaptive question selection, queue for submission handling, stack for answer review and undo, hashing for plagiarism detection, merge sort for student ranking, binary search for topic-wise question lookup, BST for storing and searching student results, and analytics/proctoring modules for performance and suspicious activity reports.

## Points to be Noted !

- Linked list is used because questions can be dynamically added.
- Trie is used because keyword prefix search is faster than scanning every keyword.
- Dynamic programming is used because question selection is like a knapsack problem.
- Queue is used because submissions are processed in arrival order.
- Stack is used because answer review undo follows last-in-first-out.
- Hashing is used because duplicate answer patterns can be detected efficiently.
- Merge sort is used because it gives stable ranking in `O(n log n)`.
- Binary search is used only after sorting questions by topic.
- BST is used for efficient roll number search.
- Proctoring timing alerts are calculated from recorded exam activity.
- Analytics are calculated from student score records.
