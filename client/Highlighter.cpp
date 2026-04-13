#include "Highlighter.h"

#include <QColor>
#include <QFont>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

Highlighter::Highlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    QTextCharFormat kwFmt;
    kwFmt.setForeground(QColor("#569cd6"));
    kwFmt.setFontWeight(QFont::Bold);

    QStringList keywords = {
        "\\bauto\\b",     "\\bbool\\b",      "\\bbreak\\b",     "\\bcase\\b",     "\\bchar\\b",
        "\\bclass\\b",    "\\bconst\\b",     "\\bcontinue\\b",  "\\bdefault\\b",  "\\bdelete\\b",
        "\\bdo\\b",       "\\bdouble\\b",    "\\belse\\b",      "\\benum\\b",     "\\bfloat\\b",
        "\\bfor\\b",      "\\bif\\b",        "\\bint\\b",       "\\blong\\b",     "\\bnew\\b",
        "\\bnullptr\\b",  "\\bprivate\\b",   "\\bprotected\\b", "\\bpublic\\b",   "\\breturn\\b",
        "\\bshort\\b",    "\\bstatic\\b",    "\\bstruct\\b",    "\\bswitch\\b",   "\\btemplate\\b",
        "\\bthis\\b",     "\\btrue\\b",      "\\bfalse\\b",     "\\btypedef\\b",  "\\btypename\\b",
        "\\bunsigned\\b", "\\busing\\b",     "\\bvirtual\\b",   "\\bvoid\\b",     "\\bwhile\\b",
        "\\bstd\\b",      "\\bnamespace\\b", "\\boverride\\b",  "\\bexplicit\\b", "\\binline\\b"};

    // loop to add all the keywords
    for (int i = 0; i < keywords.size(); i++) {
        rules.push_back({QRegularExpression(keywords[i]), kwFmt});
    }

    QTextCharFormat strFmt;
    strFmt.setForeground(QColor("#ce9178"));
    rules.push_back({QRegularExpression("\"[^\"]*\""), strFmt});
    rules.push_back({QRegularExpression("'[^']*'"), strFmt});

    QTextCharFormat slFmt;
    slFmt.setForeground(QColor("#6a9955"));
    rules.push_back({QRegularExpression("//[^\n]*"), slFmt});

    QTextCharFormat ppFmt;
    ppFmt.setForeground(QColor("#c586c0"));
    rules.push_back({QRegularExpression("^\\s*#[^\n]*"), ppFmt});

    QTextCharFormat numFmt;
    numFmt.setForeground(QColor("#b5cea8"));
    rules.push_back({QRegularExpression("\\b[0-9]+(\\.[0-9]+)?\\b"), numFmt});

    commentFmt.setForeground(QColor("#6a9955"));
    commentStart = QRegularExpression("/\\*");
    commentEnd = QRegularExpression("\\*/");
}

void Highlighter::highlightBlock(const QString& text) {
    for (int i = 0; i < rules.size(); i++) {
        QRegularExpressionMatchIterator it = rules[i].pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rules[i].fmt);
        }
    }

    setCurrentBlockState(0);

    int startIdx = 0;
    if (previousBlockState() != 1) {
        startIdx = text.indexOf(commentStart);
    }
    while (startIdx >= 0) {
        QRegularExpressionMatch endMatch = commentEnd.match(text, startIdx);
        int commentLen;

        if (endMatch.hasMatch() == false) {
            setCurrentBlockState(1);
            commentLen = text.length() - startIdx;
        } else {
            commentLen = endMatch.capturedStart() - startIdx + endMatch.capturedLength();
        }

        setFormat(startIdx, commentLen, commentFmt);
        startIdx = text.indexOf(commentStart, startIdx + commentLen);
    }
}