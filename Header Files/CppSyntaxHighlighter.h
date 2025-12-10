#ifndef CPPSYNTAXHIGHLIGHTER_H
#define CPPSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "CppSyntaxRules.h"

class CppSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit CppSyntaxHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    CppSyntaxRules rules;

    // Formats de coloration
    QTextCharFormat keywordFormat;      // Bleu pour mots-clés
    QTextCharFormat dataTypeFormat;     // Vert foncé pour types
    QTextCharFormat preprocessorFormat; // Magenta pour directives
    QTextCharFormat numberFormat;       // Rouge pour nombres
    QTextCharFormat stringFormat;       // Orange pour chaînes
    QTextCharFormat commentFormat;      // Vert clair pour commentaires
    QTextCharFormat operatorFormat;     // Gris pour opérateurs

    // Méthodes auxiliaires
    void setupFormats();
    void highlightKeywords(const QString& text);
    void highlightNumbers(const QString& text);
    void highlightStrings(const QString& text);
    void highlightComments(const QString& text);
    void highlightPreprocessor(const QString& text);
};

#endif // CPPSYNTAXHIGHLIGHTER_H