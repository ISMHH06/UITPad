// SpellHighlighter.h
#ifndef SPELLHIGHLIGHTER_H
#define SPELLHIGHLIGHTER_H


#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "spellchecker.h"


class SpellHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT


public:
    SpellHighlighter(QTextDocument* parent, SpellChecker* checker);


protected:
    void highlightBlock(const QString& text) override;


private:
    SpellChecker* spell;
    QTextCharFormat errorFormat;
};


#endif // SPELLHIGHLIGHTER_H
