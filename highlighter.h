#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "spellchecker.h"

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent, SpellChecker *checker);

protected:
    // C'est cette fonction qui est appelée à chaque frappe de clavier
    void highlightBlock(const QString &text) override;

private:
    SpellChecker *spellChecker;
    QTextCharFormat errorFormat; // Le style (souligné rouge)
};

#endif // HIGHLIGHTER_H
