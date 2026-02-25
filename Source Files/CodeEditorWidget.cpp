#include "CodeEditorWidget.h"
#include "ThemeManager.h"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>

CodeEditorWidget::CodeEditorWidget(QWidget *parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditorWidget::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
          this, &CodeEditorWidget::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
 this, &CodeEditorWidget::highlightCurrentLine);

    updateLineNumberAreaWidth(0);

    // Default colors (dark)
    applyEditorTheme(Settings::Dark);

    // Tab width = 4 spaces
    QFontMetricsF metrics(font());
    setTabStopDistance(4.0 * metrics.horizontalAdvance(' '));
}

int CodeEditorWidget::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
    ++digits;
    }
    // Minimum 3 digits width, plus left/right padding
    digits = qMax(digits, 3);
    int space = 16 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditorWidget::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditorWidget::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
 lineNumberArea->scroll(0, dy);
    else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditorWidget::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
   lineNumberAreaWidth(), cr.height()));
}

void CodeEditorWidget::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
   selection.format.setBackground(currentLineBg);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
    extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditorWidget::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), lineNumBg);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    int currentBlock = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
    QString number = QString::number(blockNumber + 1);

  if (blockNumber == currentBlock) {
       painter.setPen(lineNumActiveFg);
      QFont boldFont = painter.font();
        boldFont.setWeight(QFont::DemiBold);
                painter.setFont(boldFont);
 } else {
     painter.setPen(lineNumFg);
             QFont normalFont = painter.font();
              normalFont.setWeight(QFont::Normal);
                painter.setFont(normalFont);
            }

            painter.drawText(0, top, lineNumberArea->width() - 8,
            fontMetrics().height(),
     Qt::AlignRight | Qt::AlignVCenter, number);
    }

  block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditorWidget::applyEditorTheme(Settings::AppTheme theme)
{
    currentTheme = theme;
    auto c = ThemeManager::getColors(theme);

    lineNumBg       = QColor(c.lineNumberBg);
    lineNumFg       = QColor(c.lineNumberFg);
    lineNumActiveFg = QColor(c.lineNumberActiveFg);
    currentLineBg   = QColor(c.editorLineHighlight);

    setStyleSheet(ThemeManager::getEditorStyleSheet(theme));

    // Update tab stop for current font
    QFontMetricsF metrics(font());
    setTabStopDistance(4.0 * metrics.horizontalAdvance(' '));

    highlightCurrentLine();
    lineNumberArea->update();
}
