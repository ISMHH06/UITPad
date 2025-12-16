#ifndef CPPSYNTAXRULES_H
#define CPPSYNTAXRULES_H

#include <QStringList>
#include <QString>
#include <QSet>

class CppSyntaxRules {
public:
    CppSyntaxRules();

    // Charger les règles depuis des fichiers
    bool loadFromFiles();
    bool loadFromResources(); // Depuis qrc

    // Vérifier les catégories
    bool isKeyword(const QString& word) const;
    bool isDataType(const QString& word) const;
    bool isPreprocessor(const QString& word) const;
    bool isOperator(const QString& word) const;

    // Getters
    const QSet<QString>& getKeywords() const { return keywords; }
    const QSet<QString>& getDataTypes() const { return dataTypes; }
    const QSet<QString>& getPreprocessors() const { return preprocessors; }
    const QSet<QString>& getOperators() const { return operators; }

    // NOUVEAU : Détection automatique de code C++
    static bool isCppCode(const QString& text);

private:
    QSet<QString> keywords;       // Utiliser QSet pour recherche O(1)
    QSet<QString> dataTypes;
    QSet<QString> preprocessors;
    QSet<QString> operators;

    // Chargement depuis fichiers
    bool loadKeywordsFromFile(const QString& filePath);
    bool loadDataTypesFromFile(const QString& filePath);
    bool loadPreprocessorsFromFile(const QString& filePath);
    bool loadOperatorsFromFile(const QString& filePath);

    // Fallback : règles par défaut
    void initializeDefaultRules();

    // Helper
    QSet<QString> loadWordsFromFile(const QString& filePath);
};

#endif // CPPSYNTAXRULES_H
