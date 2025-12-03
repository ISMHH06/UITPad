// SpellHighlighter.cpp
#include "spellhighlighter.h"


SpellHighlighter::SpellHighlighter(QTextDocument* parent, SpellChecker* checker)
    : QSyntaxHighlighter(parent), spell(checker)
{
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
}


void SpellHighlighter::highlightBlock(const QString& text)
{
    if (!spell || !spell->isValid()) return;


    QRegularExpression wordRegex("\\b[\x41-\x5A\x61-\x7A]+\\b");
    auto matches = wordRegex.globalMatch(text);


    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString word = match.captured();


        if (!spell->check(word)) {
            setFormat(match.capturedStart(), match.capturedLength(), errorFormat);
        }
    }
}
