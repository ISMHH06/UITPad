#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include <QString>
#include <QStringList>
#include <QSet>

class SpellChecker
{
public:
    SpellChecker(const QString& dictionaryPath);

    bool isValid() const;
    bool check(const QString& word) const;
    QStringList suggest(const QString& word, int maxSuggestions = 5) const;

private:
    QSet<QString> dictionary;
    bool valid;

    int levenshtein(const QString& s1, const QString& s2) const;
};

#endif // SPELLCHECKER_H
