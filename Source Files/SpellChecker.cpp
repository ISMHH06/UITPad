#include "SpellChecker.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>
#include <vector>

SpellChecker::SpellChecker(const QString& dictionaryPath) {
    QFile file(dictionaryPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Erreur dico :" << dictionaryPath;
        valid = false;
        return;
    }

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8); // Pour gérer les accents
#else
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            dictionary.insert(line);
        }
    }
    file.close();
    valid = !dictionary.isEmpty();
}

bool SpellChecker::isValid() const { return valid; }

bool SpellChecker::check(const QString& word) const {
    if (!valid) return true;
    // On vérifie le mot tel quel, OU le mot en minuscules, OU avec une majuscule
    if (dictionary.contains(word)) return true;
    if (dictionary.contains(word.toLower())) return true;

    // Gestion simple des majuscules (ex: "Paris")
    QString titleCase = word.toLower();
    if (!titleCase.isEmpty()) titleCase[0] = titleCase[0].toUpper();
    return dictionary.contains(titleCase);
}

int SpellChecker::levenshtein(const QString& s1, const QString& s2) const {
    const int m = s1.length();
    const int n = s2.length();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({ dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost });
        }
    }
    return dp[m][n];
}

QStringList SpellChecker::suggest(const QString& word, int maxSuggestions) const {
    if (!valid || dictionary.empty()) return {};

    std::vector<std::pair<int, QString>> candidates;
    QString wordLower = word.toLower();

    for (const QString& dictWord : dictionary) {
        // Optimisation : On ignore les mots trop différents en taille
        if (std::abs(dictWord.length() - word.length()) > 4) continue;

        // On compare en minuscules pour trouver plus de résultats
        int dist = levenshtein(wordLower, dictWord.toLower());

        // Seuil de tolérance :
        // Si mot court (<4 lettres), tolérance 1. Sinon tolérance 3.
        int threshold = (word.length() < 4) ? 1 : 3;

        if (dist <= threshold) {
            candidates.push_back({ dist, dictWord });
        }
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const std::pair<int, QString>& a, const std::pair<int, QString>& b) {
            return a.first < b.first;
        });

    QStringList suggestions;
    // On enlève les doublons potentiels
    for (int i = 0; i < std::min((int)candidates.size(), maxSuggestions); ++i) {
        if (!suggestions.contains(candidates[i].second)) {
            suggestions.append(candidates[i].second);
        }
    }
    return suggestions;
}
