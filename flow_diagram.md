# Adaptive Online Examination Platform Flow Diagram

This file contains the system flow diagram for the project and the main module interactions.

```mermaid
flowchart TD
    A[Start / Program Launch]
    A --> B[main.cpp: Initialize Modules]
    B --> B1[seedQuestions() -> QuestionBank]
    B --> B2[seedStudents() -> StudentBST + records]
    B --> C[Menu Driven Interface]

    C --> D1[Search Questions by Keyword]
    C --> D2[Take Adaptive Exam]
    C --> D3[Binary Search Questions by Topic]
    C --> D4[Submit / Display Queue]
    C --> D5[Plagiarism Check]
    C --> D6[Answer Review Stack]
    C --> D7[Student BST Search / Display]
    C --> D8[Ranking / Analytics]
    C --> D9[Proctoring Analysis]

    D1 --> E1[QuestionBank.searchByKeyword(prefix)]
    D3 --> E2[QuestionBank.getAllQuestions()]
    E2 --> E3[BinarySearch.sortAndSearch()]

    D2 --> F1[Collect student info]
    F1 --> F2[QuestionBank.getAllQuestions()]
    F2 --> F3[AdaptiveExam.selectQuestions()]
    F3 --> F4[Present questions to student]
    F4 --> F5[Record answers + timings]
    F5 --> F6[Update StudentBST & records]
    F5 --> F7[Enqueue SubmissionQueue]
    F5 --> F8[Log ExamActivity for AutoProctor]
    F6 --> G1[MergeSort / Ranking]
    F8 --> G2[AutoProctor analysis]
    F6 --> G3[PerformanceAnalytics]

    D4 --> H1[SubmissionQueue.enqueue() / display()]
    D5 --> H2[SubmissionQueue.flagPlagiarismByHash()]
    D6 --> H3[AnswerReviewStack.push()/pop()]
    D7 --> H4[StudentBST.search() / displayAll()]
    D8 --> G1
    D9 --> G2
```

## Where to find it

- The diagram is now saved in `flow_diagram.md`.
- You can open this file in VS Code and use a Mermaid preview extension to render it visually.

## Main sections

- `main.cpp` orchestrates the menu and module calls.
- `question_bank.cpp` provides question storage and keyword search.
- `adaptive_dp_sort_search.cpp` handles adaptive exam selection, ranking, and topic search.
- `submission_queue_stack.cpp` manages submission queue, plagiarism, and review history.
- `analytics_bst_proctor.cpp` stores results in a BST and performs analytics + proctoring.
