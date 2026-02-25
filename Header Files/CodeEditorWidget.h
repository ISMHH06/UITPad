#ifndef CODEEDITORWIDGET_H
#define CODEEDITORWIDGET_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QPainter>
#include <QTextBlock>
#include "Settings.h"

/**
 * CodeEditorWidget - A QPlainTextEdit subclass with line numbers and
 * current-line highlighting, designed to be used as each editor tab.
 */
class CodeEditorWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditorWidget(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int  lineNumberAreaWidth() const;
    void applyEditorTheme(Settings::AppTheme theme);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
    Settings::AppTheme currentTheme = Settings::Dark;

    // Color cache
    QColor lineNumBg;
    QColor lineNumFg;
    QColor lineNumActiveFg;
    QColor currentLineBg;
};

// ??? LineNumberArea (internal helper) ????????????????????????????
class LineNumberArea : public QWidget
{
public:
  explicit LineNumberArea(CodeEditorWidget *editor) : QWidget(editor), codeEditor(editor) {}

    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
      codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditorWidget *codeEditor;
};

#endif // CODEEDITORWIDGET_H
