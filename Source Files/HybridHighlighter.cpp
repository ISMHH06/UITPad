#include "HybridHighlighter.h"
#include <QRegularExpression>
#include <QTextCharFormat>

HybridHighlighter::HybridHighlighter(QTextDocument* parent, SpellChecker* checker)
    : QSyntaxHighlighter(parent),
    spellChecker(checker),
    syntaxEnabled(true),
    spellCheckEnabled(true),
    isDarkTheme(true),
    forceCodeMode(false) {
    setupFormats();
}

void HybridHighlighter::setupFormats() {
    if (isDarkTheme) {
        // === MODE SOMBRE (Dark) - Couleurs actuelles ===
        keywordFormat.setForeground(QColor(86, 156, 214));        // Bleu clair
        dataTypeFormat.setForeground(QColor(78, 201, 176));       // Cyan
        preprocessorFormat.setForeground(QColor(155, 155, 155));  // Gris
        numberFormat.setForeground(QColor(181, 206, 168));        // Vert clair
        stringFormat.setForeground(QColor(255, 128, 0));          // Orange
        commentFormat.setForeground(QColor(87, 166, 74));         // Vert
        operatorFormat.setForeground(QColor(80, 80, 80));         // Gris foncé
        functionFormat.setForeground(QColor(220, 220, 170));      // Jaune pâle
    }
    else {
        // === MODE CLAIR (Light) - Couleurs adaptées pour fond blanc ===
        keywordFormat.setForeground(QColor(0, 0, 255));           // Bleu pur
        dataTypeFormat.setForeground(QColor(43, 145, 175));       // Cyan foncé
        preprocessorFormat.setForeground(QColor(128, 128, 128));  // Gris moyen
        numberFormat.setForeground(QColor(9, 134, 88));           // Vert foncé
        stringFormat.setForeground(QColor(163, 21, 21));          // Rouge brique
        commentFormat.setForeground(QColor(0, 128, 0));           // Vert foncé
        operatorFormat.setForeground(QColor(0, 0, 0));            // Noir
        functionFormat.setForeground(QColor(121, 94, 38));        // Marron/or
    }

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

// NOUVEAU : Changer le thème
void HybridHighlighter::setTheme(bool isDark) {
    isDarkTheme = isDark;
    setupFormats();
    rehighlight();  // Réappliquer la coloration avec les nouvelles couleurs
}

// NOUVEAU : Forcer le mode code
void HybridHighlighter::setForceCodeMode(bool force) {
    forceCodeMode = force;
    rehighlight();  // Réappliquer la coloration
}

// ============ DÉTECTION INTELLIGENTE LIGNE PAR LIGNE ============
bool HybridHighlighter::isCodeLine(const QString& text) const {
    QString trimmed = text.trimmed();

    // Lignes vides ou très courtes = texte normal
    if (trimmed.isEmpty() || trimmed.length() < 2) {
 return false;
    }

    int codeScore = 0;

  // ── Immediate certainties (return true) ────────────────────

    // 1. Directives préprocesseur
    if (trimmed.startsWith("#include") || trimmed.startsWith("#define") ||
     trimmed.startsWith("#ifdef") || trimmed.startsWith("#ifndef") ||
        trimmed.startsWith("#pragma") || trimmed.startsWith("#endif") ||
        trimmed.startsWith("#else") || trimmed.startsWith("#undef")) {
      return true;
    }

    // 2. Commentaires C++
    if (trimmed.startsWith("//") || trimmed.startsWith("/*") || trimmed.startsWith("*")) {
     return true;
    }

    // 3. Déclarations de fonctions/classes/using
    if (QRegularExpression("^(class|struct|enum|namespace|using)\\s+\\w+").match(trimmed).hasMatch() ||
        QRegularExpression("^template\\s*<").match(trimmed).hasMatch()) {
        return true;
    }

    // 4. Access specifiers (public:, private:, protected:)
    if (QRegularExpression("^(public|private|protected)\\s*:").match(trimmed).hasMatch()) {
   return true;
    }

    // 5. Accolades seules ou lignes terminant/commençant par accolades
    if (trimmed == "{" || trimmed == "}" || trimmed == "};" ||
        trimmed.endsWith("{") || trimmed.startsWith("}")) {
      return true;
    }

    // 6. Scope resolution (::)
    if (trimmed.contains("::")) {
    return true;
    }

    // 7. Stream operators (<< or >>) — very strong C++ indicator
    if (trimmed.contains("<<") || trimmed.contains(">>")) {
        return true;
    }

    // 8. Appel de méthode : object.method()
    if (trimmed.contains(QRegularExpression("\\w+\\.\\w+\\s*\\("))) {
        return true;
}

    // 9. Pointeur arrow (->)
    if (trimmed.contains("->")) {
        return true;
    }

    // 10. Constructor initializer list pattern:  ) : member(val)
    if (trimmed.contains(QRegularExpression("\\)\\s*:\\s*\\w+\\s*\\("))) {
     return true;
    }

    // ── Scoring-based detection ─────────────────────────────────

    // 11. Instructions se terminant par ;
    if (trimmed.endsWith(";") && trimmed.length() > 3) {
        codeScore += 3;
    }

    // 12. Syntaxe typique : fonction() { ou fonction();
    if (trimmed.contains(QRegularExpression("\\w+\\s*\\([^)]*\\)\\s*[{;]?"))) {
        // Only count if there are parentheses with content that looks like code
        if (trimmed.contains("(") && trimmed.contains(")")) {
        codeScore += 2;
    }
    }

    // 13. Increment/decrement operators (++ or --)
    if (trimmed.contains(QRegularExpression("\\w+\\s*\\+\\+")) ||
        trimmed.contains(QRegularExpression("\\+\\+\\s*\\w+")) ||
        trimmed.contains(QRegularExpression("\\w+\\s*--")) ||
   trimmed.contains(QRegularExpression("--\\s*\\w+"))) {
        codeScore += 2;
    }

    // 14. Control keywords in code context (followed by '(' or '{' or ';')
    QStringList controlKeywords = { "if", "else", "while", "for", "do", "switch",
           "case", "return", "break", "continue", "default" };
    for (const QString& kw : controlKeywords) {
        // keyword followed by ( or { or ;
        QRegularExpression contextualKw("\\b" + kw + "\\b\\s*[({;]");
   if (contextualKw.match(trimmed).hasMatch()) {
     codeScore += 3;
            break;
        }
        // "return <value>;" pattern
        if (kw == "return" && QRegularExpression("\\breturn\\b\\s+.+;$").match(trimmed).hasMatch()) {
      codeScore += 3;
      break;
        }
        // "else {" or "else if"
        if (kw == "else" && QRegularExpression("\\belse\\b\\s*[{i]").match(trimmed).hasMatch()) {
            codeScore += 3;
     break;
        }
        // "break;" or "continue;" standalone
        if ((kw == "break" || kw == "continue") &&
      QRegularExpression("^\\s*" + kw + "\\s*;").match(text).hasMatch()) {
            codeScore += 3;
    break;
        }
        // "case <value>:" or "default:"
    if (kw == "case" && QRegularExpression("\\bcase\\b\\s+.+:").match(trimmed).hasMatch()) {
        codeScore += 3;
       break;
        }
        if (kw == "default" && QRegularExpression("^\\s*default\\s*:").match(trimmed).hasMatch()) {
   codeScore += 3;
  break;
        }
    }

    // 15. Déclarations de variables (type followed by identifier)
    QStringList dataTypes = { "int", "float", "double", "char", "bool", "void",
              "string", "auto", "const", "static", "unsigned",
 "long", "short", "size_t", "nullptr" };
    for (const QString& type : dataTypes) {
        if (QRegularExpression("\\b" + type + "\\s+\\w+").match(trimmed).hasMatch()) {
     codeScore += 3;
            break;
    }
    }

// 16. Custom types (PascalCase followed by identifier or parens)
 if (QRegularExpression("^[A-Z][a-z]\\w+\\s+\\w+").match(trimmed).hasMatch() ||
        QRegularExpression("^[A-Z][a-z]\\w+\\s*\\(").match(trimmed).hasMatch() ||
      QRegularExpression("^~[A-Z]\\w+\\s*\\(").match(trimmed).hasMatch()) {
   codeScore += 2;
    }

    // 17. Compound assignment operators (+=, -=, *=, /=, ==, !=)
if (trimmed.contains(QRegularExpression("\\w+\\s*([+\\-*/]=|==|!=)\\s*"))) {
        codeScore += 2;
    }

    // 18. Array access or template syntax
    if (trimmed.contains(QRegularExpression("\\w+\\[.*\\]")) ||
        trimmed.contains(QRegularExpression("\\w+<\\w+>"))) {
  codeScore += 2;
    }

    // 19. String or char literals present
    if (trimmed.contains(QRegularExpression("\"[^\"]*\"")) ||
trimmed.contains(QRegularExpression("'[^']*'"))) {
  codeScore += 1;
    }

    // ── Natural language penalty ──────────────────────────────────
    // Only apply penalty when score is borderline and line looks like prose
    QStringList words = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (words.size() >= 3 && codeScore > 0 && codeScore < 6) {
    // Count how many words contain code-like punctuation or are code keywords
        int codeIndicators = 0;

    QSet<QString> allCodeWords;
        for (const QString& kw : controlKeywords) allCodeWords.insert(kw);
    for (const QString& dt : dataTypes) allCodeWords.insert(dt);

 for (const QString& w : words) {
            // Check for code punctuation in the original word (before stripping)
          if (w.contains("(") || w.contains(")") || w.contains(";") ||
 w.contains("{") || w.contains("}") || w.contains("::") ||
    w.contains("<<") || w.contains(">>") || w.contains("++") ||
       w.contains("--") || w.contains("->") || w.contains("=") ||
              w.contains("[") || w.contains("]")) {
    codeIndicators++;
   continue;
       }
       // Check if it's a known code word
     QString clean = w;
   clean.remove(QRegularExpression("[^A-Za-z0-9_]"));
            if (!clean.isEmpty() && allCodeWords.contains(clean.toLower())) {
                codeIndicators++;
      }
     }

        // If very few words have code characteristics, this is likely prose
        double codeRatio = static_cast<double>(codeIndicators) / words.size();
        if (codeRatio < 0.25) {
            codeScore -= 3;  // Strong penalty: almost certainly prose
      }
 }

    // Threshold: need a score of at least 3 to be considered code
    return codeScore >= 3;
}

// ============ HIGHLIGHT BLOCK PRINCIPAL ============
void HybridHighlighter::highlightBlock(const QString& text) {
    if (text.isEmpty()) {
        return;
    }

    // Si forceCodeMode est activé (fichiers .cpp/.h), toujours colorer
    if (forceCodeMode && syntaxEnabled) {
        highlightCppSyntax(text);
        return;
    }

    // Sinon, déterminer si cette ligne est du code ou du texte
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