#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include <QString>
#include <QStringList>
#include <QSet>

class SpellChecker
{
public:
    // Constructeur : charge dictionary.txt
    SpellChecker(const QString& dictionaryPath);

    // Vérifie si le dictionnaire chargé avec succès
    bool isValid() const;

    // Vérifie si un mot existe dans le dictionnaire
    bool check(const QString& word) const;

    // Propose des suggestions basées sur la distance de Levenshtein
    QStringList suggest(const QString& word, int maxSuggestions = 5) const;

private:
    QSet<QString> dictionary;   // ensemble des mots
    bool valid;

    int levenshtein(const QString& s1, const QString& s2) const;
};

#endif // SPELLCHECKER_H
