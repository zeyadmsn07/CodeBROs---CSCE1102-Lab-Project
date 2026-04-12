#include "Highlighter.h"
#include <QFont>
#include <QColor>

Highlighter::Highlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    // ── keywords (blue) ──────────────────────────────────────
    QTextCharFormat kwFmt;
    kwFmt.setForeground(QColor("#569cd6"));
    kwFmt.setFontWeight(QFont::Bold);

    const QStringList keywords = {
        "\\bauto\\b",      "\\bbool\\b",      "\\bbreak\\b",
        "\\bcase\\b",      "\\bchar\\b",       "\\bclass\\b",
        "\\bconst\\b",     "\\bcontinue\\b",   "\\bdefault\\b",
        "\\bdelete\\b",    "\\bdo\\b",         "\\bdouble\\b",
        "\\belse\\b",      "\\benum\\b",       "\\bfloat\\b",
        "\\bfor\\b",       "\\bif\\b",         "\\bint\\b",
        "\\blong\\b",      "\\bnew\\b",        "\\bnullptr\\b",
        "\\bprivate\\b",   "\\bprotected\\b",  "\\bpublic\\b",
        "\\breturn\\b",    "\\bshort\\b",      "\\bstatic\\b",
        "\\bstruct\\b",    "\\bswitch\\b",     "\\btemplate\\b",
        "\\bthis\\b",      "\\btrue\\b",       "\\bfalse\\b",
        "\\btypedef\\b",   "\\btypename\\b",   "\\bunsigned\\b",
        "\\busing\\b",     "\\bvirtual\\b",    "\\bvoid\\b",
        "\\bwhile\\b",     "\\bstd\\b",        "\\bnamespace\\b",
        "\\boverride\\b",  "\\bexplicit\\b",   "\\binline\\b"
    };
    for (const auto& kw : keywords)
        rules.push_back({ QRegularExpression(kw), kwFmt });

    // ── string literals (orange) ─────────────────────────────
    QTextCharFormat strFmt;
    strFmt.setForeground(QColor("#ce9178"));
    rules.push_back({ QRegularExpression("\"[^\"]*\""), strFmt });
    rules.push_back({ QRegularExpression("'[^']*'"),    strFmt });

    // ── single-line comments (green) ─────────────────────────
    QTextCharFormat slFmt;
    slFmt.setForeground(QColor("#6a9955"));
    rules.push_back({ QRegularExpression("//[^\n]*"), slFmt });

    // ── preprocessor lines: #include, #define … (purple) ─────
    QTextCharFormat ppFmt;
    ppFmt.setForeground(QColor("#c586c0"));
    rules.push_back({ QRegularExpression("^\\s*#[^\n]*"), ppFmt });

    // ── numbers (light green) ────────────────────────────────
    QTextCharFormat numFmt;
    numFmt.setForeground(QColor("#b5cea8"));
    rules.push_back({ QRegularExpression("\\b[0-9]+(\\.[0-9]+)?\\b"), numFmt });

    // ── multi-line comment format (used in highlightBlock) ───
    commentFmt.setForeground(QColor("#6a9955"));
    commentStart = QRegularExpression("/\\*");
    commentEnd   = QRegularExpression("\\*/");
}

void Highlighter::highlightBlock(const QString& text)
{
    // Apply all single-line rules first
    for (const auto& rule : rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.fmt);
        }
    }

    // ── multi-line comment handling ───────────────────────────
    // previousBlockState() == 1 means the previous line was inside a /* comment */
    setCurrentBlockState(0);

    int startIdx = 0;
    if (previousBlockState() != 1)
        startIdx = text.indexOf(commentStart);

    while (startIdx >= 0) {
        auto endMatch  = commentEnd.match(text, startIdx);
        int  commentLen;

        if (!endMatch.hasMatch()) {
            // Comment continues onto the next line
            setCurrentBlockState(1);
            commentLen = text.length() - startIdx;
        } else {
            commentLen = endMatch.capturedStart() - startIdx
                         + endMatch.capturedLength();
        }

        setFormat(startIdx, commentLen, commentFmt);
        startIdx = text.indexOf(commentStart, startIdx + commentLen);
    }
}