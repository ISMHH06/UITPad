#include "spellchecker.h"
#include <QFile>
#include <QTextStream>
#include <algorithm>

SpellChecker::SpellChecker(const QString& dictionaryPath)
    : valid(false)
{
    QFile file(dictionaryPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        valid = false;
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    while (!in.atEnd()) {
        QString word = in.readLine().trimmed().toLower();
        if (!word.isEmpty()) {
            dictionary.insert(word);
        }
    }

    file.close();
    valid = !dictionary.isEmpty();
}

bool SpellChecker::isValid() const
{
    return valid;
}

bool SpellChecker::check(const QString& word) const
{
    return dictionary.contains(word.toLower());
}

QStringList SpellChecker::suggest(const QString& word, int maxSuggestions) const
{
    QStringList suggestions;

    if (!valid)
        return suggestions;

    QString w = word.toLower();

    // Calcul de la distance de Levenshtein pour chaque mot
    QList<QPair<int, QString>> scored;

    for (const QString& dictWord : dictionary) {
        int score = levenshtein(w, dictWord);
        scored.append({score, dictWord});
    }

    // Trier par distance
    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    // Garder les premières suggestions
    for (int i = 0; i < std::min(maxSuggestions, scored.size()); ++i) {
        suggestions.append(scored[i].second);
    }

    return suggestions;
}

// Distance de Levenshtein (édition minimale)
int SpellChecker::levenshtein(const QString& s1, const QString& s2) const
{
    int n = s1.length();
    int m = s2.length();

    QVector<QVector<int>> dp(n + 1, QVector<int>(m + 1));

    for (int i = 0; i <= n; ++i) dp[i][0] = i;
    for (int j = 0; j <= m; ++j) dp[0][j] = j;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

            dp[i][j] = std::min({
                dp[i - 1][j] + 1,     // suppression
                dp[i][j - 1] + 1,     // insertion
                dp[i - 1][j - 1] + cost // substitution
            });
        }
    }

    return dp[n][m];
}
