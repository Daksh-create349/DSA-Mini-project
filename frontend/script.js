// ============================================================
// ExamOS Frontend — Talks to C++ HTTP Backend
// ============================================================
const API = '';  // same origin

// ── Navigation ──
document.querySelectorAll('.nav-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
        btn.classList.add('active');
        const panel = document.getElementById('panel-' + btn.dataset.panel);
        if (panel) panel.classList.add('active');

        // Auto-load data for certain panels
        const p = btn.dataset.panel;
        if (p === 'questions') loadQuestions();
        if (p === 'submissions') loadSubmissions();
        if (p === 'students') loadStudents();
    });
});

// ── Helpers ──
async function get(url) {
    const r = await fetch(API + url);
    return r.json();
}
async function post(url, data) {
    const r = await fetch(API + url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
    });
    return r.json();
}

function diffStars(d) {
    return '★'.repeat(d) + '☆'.repeat(5 - d);
}

function statusHTML(msg, type) {
    return `<div class="status-msg ${type}">${msg}</div>`;
}

// ── Question Card Builder ──
function buildQCard(q, showAnswer = true) {
    let optHtml = q.options.map((o, i) => {
        let cls = 'q-opt';
        if (showAnswer && i === q.answer) cls += ' correct';
        return `<div class="${cls}">${i}. ${o}${showAnswer && i === q.answer ? ' ✓' : ''}</div>`;
    }).join('');

    let kwHtml = (q.keywords || []).map(k => `<span class="q-kw">${k}</span>`).join('');

    return `<div class="q-card">
        <div class="q-card-top">
            <span class="q-id">Q${q.id}</span>
            <span class="q-topic">${q.topic}</span>
            <span class="q-diff">${diffStars(q.difficulty)}</span>
        </div>
        <div class="q-text">${q.text}</div>
        <div class="q-options">${optHtml}</div>
        ${kwHtml ? `<div class="q-keywords">${kwHtml}</div>` : ''}
    </div>`;
}

// ══════════════════════════════════════
// 1. QUESTION BANK
// ══════════════════════════════════════
async function loadQuestions() {
    const qs = await get('/api/questions');
    document.getElementById('questions-list').innerHTML =
        qs.length ? qs.map(q => buildQCard(q)).join('') : '<p style="color:var(--text-muted)">No questions found.</p>';
}
// Load on startup
loadQuestions();

// ── Theme Toggle ──
const themeToggle = document.getElementById('theme-toggle');
if (themeToggle) {
    themeToggle.addEventListener('click', () => {
        const current = document.documentElement.getAttribute('data-theme');
        const next = current === 'dark' ? 'light' : 'dark';
        document.documentElement.setAttribute('data-theme', next);
        localStorage.setItem('examos-theme', next);
    });
    const saved = localStorage.getItem('examos-theme');
    if (saved) {
        document.documentElement.setAttribute('data-theme', saved);
    } else if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
        document.documentElement.setAttribute('data-theme', 'dark');
    }
}

// ══════════════════════════════════════
// 2. KEYWORD SEARCH
// ══════════════════════════════════════
document.getElementById('search-btn').addEventListener('click', async () => {
    const kw = document.getElementById('search-input').value.trim();
    if (!kw) return;
    const qs = await get('/api/questions/search?keyword=' + encodeURIComponent(kw));
    document.getElementById('search-results').innerHTML =
        qs.length ? qs.map(q => buildQCard(q)).join('')
            : '<p style="color:var(--text-muted)">No questions matched.</p>';
});
document.getElementById('search-input').addEventListener('keydown', e => {
    if (e.key === 'Enter') document.getElementById('search-btn').click();
});

// ══════════════════════════════════════
// 3. TOPIC SEARCH (Binary Search)
// ══════════════════════════════════════
document.getElementById('topic-btn').addEventListener('click', async () => {
    const topic = document.getElementById('topic-select').value;
    if (!topic) return;
    const qs = await get('/api/questions/topic?topic=' + encodeURIComponent(topic));
    document.getElementById('topic-results').innerHTML =
        qs.length ? qs.map(q => buildQCard(q)).join('')
            : '<p style="color:var(--text-muted)">No questions found for this topic.</p>';
});

// ══════════════════════════════════════
// 4. ADAPTIVE EXAM (DP)
// ══════════════════════════════════════
let examQuestions = [];

document.getElementById('exam-start-btn').addEventListener('click', async () => {
    const level = parseInt(document.getElementById('exam-level').value) || 3;
    const time = parseInt(document.getElementById('exam-time').value) || 300;

    const data = await post('/api/exam/start', { studentLevel: level, timeLimit: time });
    examQuestions = data.questions || [];

    if (examQuestions.length === 0) {
        document.getElementById('exam-questions').innerHTML = statusHTML('No suitable questions found for this level.', 'error');
        document.getElementById('exam-questions').classList.remove('hidden');
        return;
    }

    let html = '';
    examQuestions.forEach((q, idx) => {
        html += `<div class="exam-q" data-qid="${q.id}">
            <div class="exam-q-header">
                <span class="q-id">Q${q.id}</span>
                <span class="q-topic">${q.topic}</span>
                <span class="q-diff">${diffStars(q.difficulty)}</span>
            </div>
            <div class="exam-q-text">${q.text}</div>
            <div class="exam-opts">
                ${q.options.map((o, i) => `
                    <label class="exam-opt-label" id="opt-${idx}-${i}">
                        <input type="radio" name="eq-${idx}" value="${i}">
                        ${i}. ${o}
                    </label>
                `).join('')}
            </div>
        </div>`;
    });
    html += `<button class="btn-primary" id="exam-submit-btn" style="margin-top:0.5rem;">Submit Exam</button>`;

    const qDiv = document.getElementById('exam-questions');
    qDiv.innerHTML = html;
    qDiv.classList.remove('hidden');
    document.getElementById('exam-result').classList.add('hidden');
    document.getElementById('exam-setup').style.display = 'none';

    // Radio selection highlight
    qDiv.querySelectorAll('input[type="radio"]').forEach(r => {
        r.addEventListener('change', () => {
            const name = r.name;
            document.querySelectorAll(`input[name="${name}"]`).forEach(rr => {
                rr.closest('.exam-opt-label').classList.remove('selected');
            });
            r.closest('.exam-opt-label').classList.add('selected');
        });
    });

    // Submit handler
    document.getElementById('exam-submit-btn').addEventListener('click', submitExam);
});

async function submitExam() {
    const roll = parseInt(document.getElementById('exam-roll').value) || 0;
    const name = document.getElementById('exam-name').value.trim();
    const level = parseInt(document.getElementById('exam-level').value) || 3;

    if (!roll || !name) {
        alert('Please fill roll number and name before starting the exam.');
        return;
    }

    const answers = [];
    const questionIds = [];
    examQuestions.forEach((q, idx) => {
        const selected = document.querySelector(`input[name="eq-${idx}"]:checked`);
        answers.push(selected ? parseInt(selected.value) : -1);
        questionIds.push(q.id);
    });

    const result = await post('/api/exam/submit', {
        roll, name, examId: 501, level, answers, questionIds
    });

    // Highlight correct/wrong
    examQuestions.forEach((q, idx) => {
        const correctAns = result.correctAnswers[idx];
        const userAns = answers[idx];
        for (let i = 0; i < 4; i++) {
            const label = document.getElementById(`opt-${idx}-${i}`);
            if (!label) continue;
            if (i === correctAns) label.classList.add('result-correct');
            else if (i === userAns && userAns !== correctAns) label.classList.add('result-wrong');
            label.querySelector('input').disabled = true;
        }
    });

    document.getElementById('exam-submit-btn').disabled = true;
    document.getElementById('exam-submit-btn').textContent = 'Submitted';

    const resDiv = document.getElementById('exam-result');
    const levelChanged = result.nextLevel !== result.currentLevel;
    const levelArrow = levelChanged ? '→' : '=';
    const levelColor = result.nextLevel > result.currentLevel ? 'var(--success)' : result.nextLevel < result.currentLevel ? 'var(--danger)' : 'var(--text-muted)';
    resDiv.innerHTML = `<div class="result-card">
        <div class="result-score">${result.score.toFixed(1)}%</div>
        <div class="result-detail">${result.correct} / ${result.total} correct &bull; Attempt #${result.attempt}</div>
        <div class="result-detail" style="margin-top:0.5rem;">
            <strong style="color:${levelColor}">
                [Adaptive] Difficulty: ${result.currentLevel} ${levelArrow} ${result.nextLevel}
            </strong>
        </div>
        <div class="result-detail" style="font-size:0.8rem;margin-top:0.25rem;color:var(--text-muted)">
            Next exam recommended at level ${result.nextLevel}
        </div>
        <button class="btn-primary" style="margin-top:1rem;" onclick="resetExam()">Take Another Exam</button>
    </div>`;
    resDiv.classList.remove('hidden');
}

function resetExam() {
    document.getElementById('exam-setup').style.display = '';
    document.getElementById('exam-questions').classList.add('hidden');
    document.getElementById('exam-questions').innerHTML = '';
    document.getElementById('exam-result').classList.add('hidden');
    examQuestions = [];
}
window.resetExam = resetExam;

// ══════════════════════════════════════
// 5. ANSWER REVIEW STACK
// ══════════════════════════════════════
document.getElementById('review-push-btn').addEventListener('click', async () => {
    const qId = parseInt(document.getElementById('review-qid').value) || 0;
    const prev = parseInt(document.getElementById('review-prev').value);
    const newA = parseInt(document.getElementById('review-new').value);
    const note = document.getElementById('review-note').value;
    if (!qId) return;
    const r = await post('/api/review/push', { questionId: qId, prevAnswer: prev, newAnswer: newA, note });
    document.getElementById('review-status').innerHTML =
        statusHTML(`Pushed review for Q${qId}: ${prev} → ${newA}. Stack size: ${r.size}`, 'success');
});

document.getElementById('review-pop-btn').addEventListener('click', async () => {
    const r = await post('/api/review/pop', {});
    if (r.empty) {
        document.getElementById('review-status').innerHTML = statusHTML('Stack is empty — nothing to undo.', 'error');
    } else {
        document.getElementById('review-status').innerHTML =
            statusHTML(`Undone: Q${r.questionId} reverted ${r.newAnswer} → ${r.prevAnswer}`, 'info');
    }
});

// ══════════════════════════════════════
// 6. ADD SUBMISSION
// ══════════════════════════════════════
document.getElementById('sub-add-btn').addEventListener('click', async () => {
    const sid = parseInt(document.getElementById('sub-sid').value) || 0;
    const eid = parseInt(document.getElementById('sub-eid').value) || 0;
    const name = document.getElementById('sub-name').value.trim();
    const answersStr = document.getElementById('sub-answers').value.trim();
    const answers = answersStr.split(',').map(s => parseInt(s.trim())).filter(n => !isNaN(n));

    if (!sid || !eid || !name || answers.length === 0) {
        document.getElementById('sub-status').innerHTML = statusHTML('Please fill all fields correctly.', 'error');
        return;
    }

    const r = await post('/api/submissions/add', { studentId: sid, examId: eid, name, answers });
    document.getElementById('sub-status').innerHTML = r.success
        ? statusHTML(`Submission added: ${name} (ID:${sid}, Exam:${eid})`, 'success')
        : statusHTML('Failed to add submission.', 'error');
});

// ══════════════════════════════════════
// 7. SUBMISSIONS QUEUE
// ══════════════════════════════════════
async function loadSubmissions() {
    const subs = await get('/api/submissions');
    if (subs.length === 0) {
        document.getElementById('submissions-list').innerHTML = '<p style="color:var(--text-muted)">(Queue is empty)</p>';
        return;
    }
    let html = `<table class="data-table">
        <thead><tr><th>#</th><th>Name</th><th>ID</th><th>Exam</th><th>Source</th><th>Answers</th><th>Flag</th></tr></thead><tbody>`;
    subs.forEach((s, i) => {
        html += `<tr>
            <td>${i + 1}</td>
            <td>${s.name}</td>
            <td>${s.studentId}</td>
            <td>${s.examId}</td>
            <td>${s.source}</td>
            <td style="font-family:var(--font-mono);font-size:0.8rem;">${s.answers.join(', ')}</td>
            <td>${s.plagiarism ? '<span class="flag-plag">⚠ FLAGGED</span>' : '—'}</td>
        </tr>`;
    });
    html += '</tbody></table>';
    document.getElementById('submissions-list').innerHTML = html;
}
document.getElementById('sub-refresh-btn').addEventListener('click', loadSubmissions);

// ══════════════════════════════════════
// 8. PLAGIARISM CHECK
// ══════════════════════════════════════
document.getElementById('plag-btn').addEventListener('click', async () => {
    const r = await post('/api/plagiarism', {});
    let html = statusHTML(`Plagiarism check complete. ${r.flagged} duplicate pattern(s) flagged.`, r.flagged > 0 ? 'error' : 'success');
    if (r.submissions && r.submissions.length) {
        html += '<div style="margin-top:0.75rem;">';
        r.submissions.forEach(s => {
            html += `<div class="q-card" style="padding:0.65rem 0.85rem;margin-bottom:0.4rem;">
                <strong>${s.name}</strong> (ID:${s.studentId}) — Answers: [${s.answers.join(', ')}]
                ${s.plagiarism ? ' <span class="flag-plag">⚠ PLAGIARISM FLAG</span>' : ' ✓ Clean'}
            </div>`;
        });
        html += '</div>';
    }
    document.getElementById('plag-results').innerHTML = html;
});

// ══════════════════════════════════════
// 9. STUDENTS BST
// ══════════════════════════════════════
async function loadStudents() {
    const students = await get('/api/students');
    if (students.length === 0) {
        document.getElementById('students-table').innerHTML = '<p style="color:var(--text-muted)">(No records)</p>';
        return;
    }
    let html = `<table class="data-table">
        <thead><tr><th>Roll No</th><th>Name</th><th>Score</th><th>Exam ID</th></tr></thead><tbody>`;
    students.forEach(s => {
        html += `<tr>
            <td style="font-family:var(--font-mono)">${s.rollNo}</td>
            <td>${s.name}</td>
            <td>${s.score.toFixed(2)}</td>
            <td>${s.examId}</td>
        </tr>`;
    });
    html += '</tbody></table>';
    document.getElementById('students-table').innerHTML = html;
}

document.getElementById('student-search-btn').addEventListener('click', async () => {
    const roll = parseInt(document.getElementById('student-roll').value) || 0;
    if (!roll) return;
    const r = await get('/api/students/search?roll=' + roll);
    if (r.found) {
        document.getElementById('student-search-result').innerHTML =
            statusHTML(`Found — <strong>${r.name}</strong> | Roll: ${r.rollNo} | Score: ${r.score.toFixed(2)} | Exam: ${r.examId}`, 'success');
    } else {
        document.getElementById('student-search-result').innerHTML =
            statusHTML(`Roll ${roll} not found in BST.`, 'error');
    }
});

// ══════════════════════════════════════
// 10. RANKINGS (Merge Sort)
// ══════════════════════════════════════
document.getElementById('ranking-btn').addEventListener('click', async () => {
    const ranked = await get('/api/ranking');
    if (ranked.length === 0) {
        document.getElementById('ranking-table').innerHTML = '<p style="color:var(--text-muted)">(No records)</p>';
        return;
    }
    let html = `<table class="data-table">
        <thead><tr><th>Rank</th><th>Roll No</th><th>Name</th><th>Score</th></tr></thead><tbody>`;
    ranked.forEach((s, i) => {
        const medal = i === 0 ? '🥇 ' : i === 1 ? '🥈 ' : i === 2 ? '🥉 ' : '';
        html += `<tr>
            <td>${medal}${s.rank}</td>
            <td style="font-family:var(--font-mono)">${s.rollNo}</td>
            <td>${s.name}</td>
            <td><strong>${s.score.toFixed(2)}</strong></td>
        </tr>`;
    });
    html += '</tbody></table>';
    document.getElementById('ranking-table').innerHTML = html;
});

// ══════════════════════════════════════
// 11. PROCTORING
// ══════════════════════════════════════
document.getElementById('proctor-btn').addEventListener('click', async () => {
    const roll = parseInt(document.getElementById('proctor-roll').value) || 0;
    if (!roll) return;
    const cp = document.getElementById('proctor-cp').checked;
    const ts = document.getElementById('proctor-ts').checked;
    const fa = document.getElementById('proctor-fa').checked;

    const r = await post('/api/proctoring', { roll, copyPaste: cp, tabSwitch: ts, faceAbsent: fa });

    if (r.error) {
        document.getElementById('proctor-results').innerHTML = statusHTML(r.error + '. Take an exam first (option 4).', 'error');
        return;
    }

    // Recorded Exam Timing (same as terminal output)
    let html = '';
    if (r.timing && r.timing.length) {
        html += `<div class="result-card" style="text-align:left;margin-bottom:0.75rem;">
            <p style="margin-bottom:0.5rem;"><strong>===== Recorded Exam Timing =====</strong></p>`;
        r.timing.forEach(t => {
            html += `<p style="font-family:var(--font-mono);font-size:0.82rem;color:var(--text-muted);margin-bottom:0.2rem;">
                Answer ${t.answer} submitted at ${t.seconds}s (${t.millis}ms)
            </p>`;
        });
        html += `<p style="margin-top:0.4rem;color:var(--success);font-size:0.82rem;">
            ${r.timingAnalyzed ? 'Timing analysis completed automatically.' : ''}
        </p></div>`;
    }

    html += `<div class="result-card" style="text-align:left;">
        <p><strong>===== Proctoring Report =====</strong></p>
        <p><strong>Student:</strong> ${r.name} (Roll: ${r.roll})</p>
        <p><strong>Alerts:</strong> ${r.alertCount}</p>
        <span class="suspicious-badge ${r.suspicious ? 'sus' : 'clean'}">
            ${r.suspicious ? '⚠ SUSPICIOUS' : '✓ CLEAN'}
        </span>
    </div>`;

    if (r.alerts && r.alerts.length) {
        html += '<div style="margin-top:0.75rem;">';
        r.alerts.forEach(a => {
            html += `<div class="alert-card">
                <span class="alert-type">${a.type}</span> — ${a.detail}
            </div>`;
        });
        html += '</div>';
    }
    document.getElementById('proctor-results').innerHTML = html;
});

// ══════════════════════════════════════
// 12. ANALYTICS
// ══════════════════════════════════════
document.getElementById('analytics-btn').addEventListener('click', async () => {
    const stats = await get('/api/analytics');
    let topicHtml = '';
    if (stats.topicWise && stats.topicWise.length) {
        topicHtml = `<div style="margin-top:1rem;">
            <h3 style="font-size:0.9rem;margin-bottom:0.5rem;color:var(--text-muted);">Topic-wise Performance</h3>
            <table class="data-table">
                <thead><tr><th>Topic</th><th>Average Score</th><th>Performance</th></tr></thead>
                <tbody>${stats.topicWise.map(t => {
                    const color = t.average >= 70 ? 'var(--success)' : t.average >= 40 ? 'var(--warning)' : 'var(--danger)';
                    const bar = Math.min(100, Math.max(0, t.average));
                    return `<tr>
                        <td><strong>${t.topic}</strong></td>
                        <td style="font-family:var(--font-mono);color:${color}">${t.average.toFixed(2)}%</td>
                        <td><div style="background:var(--bg-hover);border-radius:4px;height:8px;width:120px;border:1px solid var(--border);">
                            <div style="background:${color};height:100%;width:${bar}%;border-radius:4px;transition:width 0.3s;"></div>
                        </div></td>
                    </tr>`;
                }).join('')}</tbody>
            </table>
        </div>`;
    } else {
        topicHtml = `<p style="margin-top:1rem;color:var(--text-muted);font-size:0.85rem;">
            No topic-wise data yet. Take an exam first to generate per-topic scores.
        </p>`;
    }
    document.getElementById('analytics-results').innerHTML = `
        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-value" style="color:var(--accent)">${stats.average.toFixed(2)}</div>
                <div class="stat-label">Average Score</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" style="color:var(--success)">${stats.highest.toFixed(2)}</div>
                <div class="stat-label">Highest Score</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" style="color:var(--danger)">${stats.lowest.toFixed(2)}</div>
                <div class="stat-label">Lowest Score</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" style="color:var(--warning)">${stats.stdDev.toFixed(2)}</div>
                <div class="stat-label">Std Deviation</div>
            </div>
            <div class="stat-card">
                <div class="stat-value">${stats.count}</div>
                <div class="stat-label">Total Students</div>
            </div>
        </div>
        ${topicHtml}`;
});
