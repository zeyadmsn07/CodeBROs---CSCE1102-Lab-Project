#pragma once
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT
   public:
    explicit Highlighter(QTextDocument* parent);

   protected:
    void highlightBlock(const QString& text) override;

   private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat fmt;
    };
    QVector<Rule> rules;

    QRegularExpression commentStart;
    QRegularExpression commentEnd;
    QTextCharFormat commentFmt;
};