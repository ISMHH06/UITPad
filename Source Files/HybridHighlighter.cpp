#include "HybridHighlighter.h"
#include <QRegularExpression>
#include <QTextCharFormat>

HybridHighlighter::HybridHighlighter(QTextDocument* parent, SpellChecker* checker)
    : QSyntaxHighlighter(parent),
    spellChecker(checker),
    syntaxEnabled(true),
    spellCheckEnabled(true) {
    setupFormats();
}

void HybridHighlighter::setupFormats() {
    // Formats pour la coloration syntaxique
    keywordFormat.setForeground(QColor(86, 156, 214));

    dataTypeFormat.setForeground(QColor(78, 201, 176));

    preprocessorFormat.setForeground(QColor(155, 155, 155));

    numberFormat.setForeground(QColor(181, 206, 168));

    stringFormat.setForeground(QColor(255, 128, 0));

    commentFormat.setForeground(QColor(87, 166, 74));

    operatorFormat.setForeground(QColor(80, 80, 80));

    // Format pour les fonctions
    functionFormat.setForeground(QColor(220, 220, 170));

    // Format pour les erreurs d'orthographe (soulignement rouge ondulé)
    spellErrorFormat.setUnderlineColor(Qt::red);
    spellErrorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
}

void HybridHighlighter::setSyntaxHighlightingEnabled(bool enabled) {
    syntaxEnabled = enabled;
    rehighlight();
}

void HybridHighlighter::setSpellCheckEnabled(bool enabled) {
    spellCheckEnabled = enabled;
    rehighlight();
}

// ============ DÉTECTION INTELLIGENTE LIGNE PAR LIGNE ============
bool HybridHighlighter::isCodeLine(const QString& text) const {
    QString trimmed = text.trimmed();

    // Lignes vides ou très courtes = texte normal
    if (trimmed.isEmpty() || trimmed.length() < 2) {
        return false;
    }

    int codeScore = 0;

    // 1. Directives préprocesseur (très fort indicateur)
    if (trimmed.startsWith("#include") || trimmed.startsWith("#define") ||
        trimmed.startsWith("#ifdef") || trimmed.startsWith("#ifndef") ||
        trimmed.startsWith("#pragma")) {
        return true;
    }

    // 2. Commentaires C++ (indicateur fort)
    if (trimmed.startsWith("//") || trimmed.startsWith("/*") || trimmed.startsWith("*")) {
        return true;  // Changé de +3 à certitude
    }

    // 3. Déclarations de fonctions/classes
    if (QRegularExpression("^(class|struct|enum|namespace|template)\\s+\\w+").match(trimmed).hasMatch()) {
        return true;
    }

    // 4. Access specifiers (public:, private:, protected:)
    if (QRegularExpression("^(public|private|protected)\\s*:").match(trimmed).hasMatch()) {
        return true;
    }

    // 5. NOUVEAU : Accolades seules ou avec contenu minimal (très fort indicateur)
    if (trimmed == "{" || trimmed == "}" || trimmed == "};" ||
        trimmed.endsWith("{") || trimmed.startsWith("}")) {
        return true;
    }

    // 6. NOUVEAU : Lignes avec scope resolution (::)
    if (trimmed.contains("::")) {
        return true;
    }

    // 7. NOUVEAU : Instructions se terminant par ; (très commun en C++)
    if (trimmed.endsWith(";") && trimmed.length() > 3) {
        codeScore += 3;
    }

    // 8. Syntaxe typique : fonction()
    if (trimmed.contains(QRegularExpression("\\w+\\s*\\([^)]*\\)\\s*[{;]"))) {
        codeScore += 3;
    }

    // 9. Appel de méthode : object.method()
    if (trimmed.contains(QRegularExpression("\\w+\\.\\w+\\s*\\("))) {
        return true;
    }

    // 10. Instructions de contrôle
    QStringList controlKeywords = { "if", "else", "while", "for", "do", "switch",
                                    "case", "return", "break", "continue" };
    for (const QString& kw : controlKeywords) {
        if (QRegularExpression("\\b" + kw + "\\b").match(trimmed).hasMatch()) {
            codeScore += 3;
            break;  // Un seul suffit
        }
    }

    // 11. Déclarations de variables typiques
    QStringList dataTypes = { "int", "float", "double", "char", "bool", "void",
                             "string", "auto", "const", "static", "unsigned",
                             "long", "short" };
    for (const QString& type : dataTypes) {
        if (QRegularExpression("\\b" + type + "\\s+\\w+").match(trimmed).hasMatch()) {
            codeScore += 3;
            break;
        }
    }

    // 12. Déclarations avec types personnalisés (majuscule au début)
    if (QRegularExpression("^[A-Z]\\w+\\s+\\w+").match(trimmed).hasMatch()) {
        codeScore += 2;
    }

    // 13. Opérateurs d'affectation
    if (trimmed.contains(QRegularExpression("\\w+\\s*[=+\\-*/]?=\\s*"))) {
        codeScore += 1;
    }

    // 14. Pointeur ou flèche
    if (trimmed.contains("->") || trimmed.contains("&") || trimmed.contains("*")) {
        codeScore += 1;
    }

    // Si le score est >= 3, c'est probablement du code
    return codeScore >= 3;
}

// ============ HIGHLIGHT BLOCK PRINCIPAL ============
void HybridHighlighter::highlightBlock(const QString& text) {
    if (text.isEmpty()) {
        return;
    }

    // Déterminer si cette ligne est du code ou du texte
    bool isCode = isCodeLine(text);

    if (isCode && syntaxEnabled) {
        // LIGNE DE CODE → Appliquer la coloration syntaxique
        highlightCppSyntax(text);
    }
    else if (!isCode && spellCheckEnabled && spellChecker && spellChecker->isValid()) {
        // LIGNE DE TEXTE → Appliquer la correction orthographique
        highlightSpelling(text);
    }
}

// ============ COLORATION SYNTAXIQUE C++ ============
void HybridHighlighter::highlightCppSyntax(const QString& text) {
    // Ordre important : commentaires et strings en dernier pour tout recouvrir
    highlightPreprocessor(text);
    highlightStrings(text);        // DÉPLACÉ AVANT keywords pour protéger les strings
    highlightKeywords(text);
    highlightFunctions(text);
    highlightNumbers(text);
    highlightComments(text);       // En dernier pour tout recouvrir
}

void HybridHighlighter::highlightPreprocessor(const QString& text) {
    // Directive préprocesseur (#include, #define, etc.)
    QRegularExpression preprocessorRegex("^\\s*(#\\w+)");
    QRegularExpressionMatch match = preprocessorRegex.match(text);

    if (match.hasMatch()) {
        int start = match.capturedStart(1);
        int length = match.capturedLength(1);
        setFormat(start, length, preprocessorFormat);

        // FIX : Colorer le nom de la bibliothèque dans #include <xxx> ou #include "xxx"
        QRegularExpression includeLibRegex("#include\\s*[<\"]([^>\"]+)[>\"]");
        QRegularExpressionMatch libMatch = includeLibRegex.match(text);

        if (libMatch.hasMatch()) {
            int libStart = libMatch.capturedStart(1);
            int libLength = libMatch.capturedLength(1);
            setFormat(libStart, libLength, stringFormat);  // Orange, pas vert !
        }
    }
}

void HybridHighlighter::highlightKeywords(const QString& text) {
    QRegularExpression wordRegex("\\b[A-Za-z_][A-Za-z0-9_]*\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0);
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);

        // Vérifier si c'est déjà formaté (dans un string par exemple)
        QTextCharFormat existingFormat = format(start);
        if (existingFormat.foreground().color() == stringFormat.foreground().color()) {
            continue;  // Ne pas écraser les strings
        }

        if (rules.isKeyword(word)) {
            setFormat(start, length, keywordFormat);
        }
        else if (rules.isDataType(word)) {
            // FIX : Ne pas colorer les types qui sont dans des #include
            // Vérifier si on est dans une ligne #include
            if (!text.trimmed().startsWith("#include")) {
                setFormat(start, length, dataTypeFormat);
            }
        }
    }

    // Gérer spécifiquement public:, private:, protected: (avec deux-points)
    QRegularExpression accessSpecifierRegex("\\b(public|private|protected)(?=\\s*:)");
    QRegularExpressionMatchIterator accessIt = accessSpecifierRegex.globalMatch(text);

    while (accessIt.hasNext()) {
        QRegularExpressionMatch match = accessIt.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, keywordFormat);
    }
}

// Colorer les noms de fonctions
void HybridHighlighter::highlightFunctions(const QString& text) {
    // Regex pour capturer : nom_fonction(
    QRegularExpression functionRegex("\\b([A-Za-z_][A-Za-z0-9_]*)\\s*(?=\\()");
    QRegularExpressionMatchIterator it = functionRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString funcName = match.captured(1);
        int start = match.capturedStart(1);
        int length = match.capturedLength(1);

        // Ne pas colorer si c'est un mot-clé
        if (!rules.isKeyword(funcName) && !rules.isDataType(funcName)) {
            setFormat(start, length, functionFormat);
        }
    }
}

void HybridHighlighter::highlightNumbers(const QString& text) {
    QRegularExpression numberRegex("\\b[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?[fFlLuU]*\\b");
    QRegularExpressionMatchIterator it = numberRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, numberFormat);
    }

    QRegularExpression hexRegex("\\b0[xX][0-9A-Fa-f]+[uUlL]*\\b");
    it = hexRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, numberFormat);
    }
}

void HybridHighlighter::highlightStrings(const QString& text) {
    // Strings entre guillemets doubles
    QRegularExpression stringRegex("\"([^\"]|\\\\.)*\"");
    QRegularExpressionMatchIterator it = stringRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, stringFormat);
    }

    // Caractères entre guillemets simples
    QRegularExpression charRegex("'([^']|\\\\.)'");
    it = charRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);
        setFormat(start, length, stringFormat);
    }
}

void HybridHighlighter::highlightComments(const QString& text) {
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

// ============ CORRECTION ORTHOGRAPHIQUE ============
void HybridHighlighter::highlightSpelling(const QString& text) {
    // Extraire tous les mots
    QRegularExpression wordRegex("\\b[A-Za-zÀ-ÿ][A-Za-zÀ-ÿ']*\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0);
        int start = match.capturedStart(0);
        int length = match.capturedLength(0);

        // Vérifier l'orthographe
        if (!spellChecker->check(word)) {
            setFormat(start, length, spellErrorFormat);
        }
    }
}