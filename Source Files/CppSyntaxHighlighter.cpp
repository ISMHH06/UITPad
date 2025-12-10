#include "CppSyntaxHighlighter.h"
#include <QRegularExpression>

CppSyntaxHighlighter::CppSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    setupFormats();
}

void CppSyntaxHighlighter::setupFormats() {
    // Mots-clés : Bleu
    keywordFormat.setForeground(QColor(0, 0, 255));
    keywordFormat.setFontWeight(QFont::Bold);

    // Types de données : Vert foncé
    dataTypeFormat.setForeground(QColor(0, 128, 0));
    dataTypeFormat.setFontWeight(QFont::Bold);

    // Directives préprocesseur : Magenta
    preprocessorFormat.setForeground(QColor(128, 0, 128));
    preprocessorFormat.setFontWeight(QFont::Bold);

    // Nombres : Rouge
    numberFormat.setForeground(QColor(200, 0, 0));

    // Chaînes : Orange
    stringFormat.setForeground(QColor(255, 128, 0));

    // Commentaires : Vert clair italique
    commentFormat.setForeground(QColor(0, 180, 0));
    commentFormat.setFontItalic(true);

    // Opérateurs : Gris foncé
    operatorFormat.setForeground(QColor(80, 80, 80));
}

void CppSyntaxHighlighter::highlightBlock(const QString& text) {
    // Ordre important : commentaires en dernier pour tout recouvrir
    highlightPreprocessor(text);
    highlightKeywords(text);
    highlightNumbers(text);
    highlightStrings(text);
    highlightComments(text);
}

void CppSyntaxHighlighter::highlightPreprocessor(const QString& text) {
    // Détecte les directives préprocesseur (#include, #define, etc.)
    QRegularExpression preprocessorRegex("^\\s*(#\\w+)");
    QRegularExpressionMatch match = preprocessorRegex.match(text);

    if (match.hasMatch()) {
        int start = match.capturedStart(1);
        int length = match.capturedLength(1);
        setFormat(start, length, preprocessorFormat);
    }
}

void CppSyntaxHighlighter::highlightKeywords(const QString& text) {
    // Expression régulière pour capturer les mots
    QRegularExpression wordRegex("\\b[A-Za-z_][A-Za-z0-9_]*\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0);
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);

        // Vérifier si c'est un mot-clé
        if (rules.isKeyword(word)) {
            setFormat(start, length, keywordFormat);
        }
        // Vérifier si c'est un type de données
        else if (rules.isDataType(word)) {
            setFormat(start, length, dataTypeFormat);
        }
    }
}

void CppSyntaxHighlighter::highlightNumbers(const QString& text) {
    // Nombres entiers et décimaux
    QRegularExpression numberRegex("\\b[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?[fFlLuU]*\\b");
    QRegularExpressionMatchIterator it = numberRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, numberFormat);
    }

    // Nombres hexadécimaux (0x...)
    QRegularExpression hexRegex("\\b0[xX][0-9A-Fa-f]+[uUlL]*\\b");
    it = hexRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, numberFormat);
    }
}

void CppSyntaxHighlighter::highlightStrings(const QString& text) {
    // Chaînes entre guillemets doubles "..."
    QRegularExpression stringRegex("\"([^\"]|\\\\.)*\"");
    QRegularExpressionMatchIterator it = stringRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, stringFormat);
    }

    // Caractères entre guillemets simples '...'
    QRegularExpression charRegex("'([^']|\\\\.)'");
    it = charRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, stringFormat);
    }
}

void CppSyntaxHighlighter::highlightComments(const QString& text) {
    // Commentaires sur une ligne //...
    QRegularExpression singleLineCommentRegex("//[^\n]*");
    QRegularExpressionMatchIterator it = singleLineCommentRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, commentFormat);
    }

    // Commentaires multi-lignes /* ... */
    // Gestion d'état pour les commentaires multi-lignes
    setCurrentBlockState(0);

    QRegularExpression multiLineCommentStart("/\\*");
    QRegularExpression multiLineCommentEnd("\\*/");

    int startIndex = 0;
    if (previousBlockState() != 1) {
        QRegularExpressionMatch match = multiLineCommentStart.match(text);
        startIndex = match.hasMatch() ? match.capturedStart() : -1;
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = multiLineCommentEnd.match(text, startIndex);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;

        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, commentLength, commentFormat);

        QRegularExpressionMatch nextMatch = multiLineCommentStart.match(text, startIndex + commentLength);
        startIndex = nextMatch.hasMatch() ? nextMatch.capturedStart() : -1;
    }
}