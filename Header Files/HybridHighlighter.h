#ifndef HYBRIDHIGHLIGHTER_H
#define HYBRIDHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include "CppSyntaxRules.h"
#include "SpellChecker.h"

class HybridHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit HybridHighlighter(QTextDocument* parent,
        SpellChecker* spellChecker = nullptr);

    // Activer/désactiver les fonctionnalités
    void setSyntaxHighlightingEnabled(bool enabled);
    void setSpellCheckEnabled(bool enabled);

    bool isSyntaxHighlightingEnabled() const { return syntaxEnabled; }
    bool isSpellCheckEnabled() const { return spellCheckEnabled; }

    // NOUVEAU : Changer le thème de coloration
    void setTheme(bool isDarkMode);

    // NOUVEAU : Forcer le mode code (pour .cpp/.h)
    void setForceCodeMode(bool force);

protected:
    void highlightBlock(const QString& text) override;

private:
    CppSyntaxRules rules;
    SpellChecker* spellChecker;

    bool syntaxEnabled;
    bool spellCheckEnabled;
    bool isDarkTheme;
    bool forceCodeMode;  // NOUVEAU : Force le mode code pour les fichiers .cpp/.h

    // Formats pour la coloration syntaxique
    QTextCharFormat keywordFormat;
    QTextCharFormat dataTypeFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat functionFormat;  // NOUVEAU : Format pour les fonctions

    // Format pour le correcteur orthographique
    QTextCharFormat spellErrorFormat;

    // Méthodes de détection
    bool isCodeLine(const QString& text) const;

    // Méthodes de coloration syntaxique
    void highlightCppSyntax(const QString& text);
    void highlightKeywords(const QString& text);
    void highlightFunctions(const QString& text);  // NOUVEAU
    void highlightNumbers(const QString& text);
    void highlightStrings(const QString& text);
    void highlightComments(const QString& text);
    void highlightPreprocessor(const QString& text);

    // Méthode de correction orthographique
    void highlightSpelling(const QString& text);

    // Initialisation
    void setupFormats();
};

#endif // HYBRIDHIGHLIGHTER_H