#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent, SpellChecker *checker)
    : QSyntaxHighlighter(parent), spellChecker(checker)
{
    // On définit le style : Souligné ondulé (Wave) en rouge
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void Highlighter::highlightBlock(const QString &text)
{
    if (!spellChecker || !spellChecker->isValid()) return;

    // On utilise une expression régulière pour trouver les mots (lettres seulement)
    QRegularExpression expression("\\b[\\w]+\\b");
    QRegularExpressionMatchIterator i = expression.globalMatch(text);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured(0);

        // Si le mot est mal orthographié, on applique le format rouge
        if (!spellChecker->check(word)) {
            setFormat(match.capturedStart(), match.capturedLength(), errorFormat);
        }
    }
}
