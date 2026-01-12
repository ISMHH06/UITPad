#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <QDockWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QColor>
#include "Settings.h"

class OutputWindow : public QDockWidget
{
    Q_OBJECT

public:
    explicit OutputWindow(QWidget* parent = nullptr);

    // Append output to specific tabs
    void appendCompileOutput(const QString& text);
    void appendCompileError(const QString& text);
    void appendRunOutput(const QString& text);
    void appendRunError(const QString& text);

    // Clear specific tabs
    void clearCompileOutput();
    void clearRunOutput();
    void clearAll();

    // Tab switching
    void showCompileTab();
    void showRunTab();
    
    // NEW: Apply theme
    void applyTheme(Settings::AppTheme theme);

signals:
    void stopCompilationRequested();
    void stopExecutionRequested();

private:
    QTabWidget* tabWidget;
    QPlainTextEdit* compileOutput;
    QPlainTextEdit* runOutput;
    QPushButton* btnClearCompile;
    QPushButton* btnClearRun;
    QPushButton* btnStopCompile;
    QPushButton* btnStopRun;
    
    Settings::AppTheme currentTheme = Settings::Dark;

    void setupUI();
    void refreshTextColors(QPlainTextEdit* output);  // NEW
    QColor getTextColor(const QString& text) const;   // NEW
};

#endif // OUTPUTWINDOW_H