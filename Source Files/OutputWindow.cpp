#include "OutputWindow.h"
#include "ThemeManager.h"
#include <QVBoxLayout>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QScrollBar>
#include <QTextBlock>

OutputWindow::OutputWindow(QWidget* parent)
    : QDockWidget("OUTPUT", parent)
{
    setupUI();
}

void OutputWindow::setupUI()
{
    QWidget* mainWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    tabWidget = new QTabWidget(mainWidget);
    tabWidget->setDocumentMode(true);

    // Compilation output tab
    QWidget* compileTab = new QWidget();
    QVBoxLayout* compileLayout = new QVBoxLayout(compileTab);
    compileLayout->setContentsMargins(0, 0, 0, 0);
    compileLayout->setSpacing(0);

    compileOutput = new QPlainTextEdit(compileTab);
    compileOutput->setReadOnly(true);
    compileLayout->addWidget(compileOutput);

    QHBoxLayout* compileButtons = new QHBoxLayout();
    compileButtons->setContentsMargins(4, 4, 4, 4);
    compileButtons->setSpacing(4);
    btnStopCompile = new QPushButton("Stop", compileTab);
    btnClearCompile = new QPushButton("Clear", compileTab);
    compileButtons->addWidget(btnStopCompile);
    compileButtons->addWidget(btnClearCompile);
    compileButtons->addStretch();
    compileLayout->addLayout(compileButtons);

    connect(btnStopCompile, &QPushButton::clicked,
            this, &OutputWindow::stopCompilationRequested);
    connect(btnClearCompile, &QPushButton::clicked,
            this, &OutputWindow::clearCompileOutput);

    // Run output tab
    QWidget* runTab = new QWidget();
    QVBoxLayout* runLayout = new QVBoxLayout(runTab);
    runLayout->setContentsMargins(0, 0, 0, 0);
    runLayout->setSpacing(0);

    runOutput = new QPlainTextEdit(runTab);
    runOutput->setReadOnly(true);
    runLayout->addWidget(runOutput);

    QHBoxLayout* runButtons = new QHBoxLayout();
    runButtons->setContentsMargins(4, 4, 4, 4);
    runButtons->setSpacing(4);
    btnStopRun = new QPushButton("Stop", runTab);
    btnClearRun = new QPushButton("Clear", runTab);
    runButtons->addWidget(btnStopRun);
    runButtons->addWidget(btnClearRun);
    runButtons->addStretch();
    runLayout->addLayout(runButtons);

    connect(btnStopRun, &QPushButton::clicked,
            this, &OutputWindow::stopExecutionRequested);
    connect(btnClearRun, &QPushButton::clicked,
            this, &OutputWindow::clearRunOutput);

    tabWidget->addTab(compileTab, "Compilation");
    tabWidget->addTab(runTab, "Run");

    mainLayout->addWidget(tabWidget);
    setWidget(mainWidget);

    setMinimumHeight(120);
    
    applyTheme(Settings::Dark);
}

void OutputWindow::applyTheme(Settings::AppTheme theme)
{
    currentTheme = theme;
    
    // Use ThemeManager for consistent styling
    QString outputStyle = ThemeManager::getOutputTextStyleSheet(theme);
    QString widgetStyle = ThemeManager::getOutputWindowStyleSheet(theme);
    
    compileOutput->setStyleSheet(outputStyle);
    runOutput->setStyleSheet(outputStyle);
    widget()->setStyleSheet(widgetStyle);
    
    // Refresh existing text colors
    refreshTextColors(compileOutput);
    refreshTextColors(runOutput);
}

// NEW: Refresh all text colors in an output widget
void OutputWindow::refreshTextColors(QPlainTextEdit* output)
{
    if (!output || output->document()->isEmpty()) return;
    
    QTextDocument* doc = output->document();
    QTextCursor cursor(doc);
    
    cursor.beginEditBlock();
    
    // Go through each block (line) and recolor
    QTextBlock block = doc->begin();
    while (block.isValid()) {
        QString text = block.text();
        
        // Select the entire block
        cursor.setPosition(block.position());
        cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
        
        // Determine the appropriate color
        QTextCharFormat format;
        format.setForeground(getTextColor(text));
        
        if (text.startsWith(">>")) {
            format.setFontWeight(QFont::Bold);
        }
        
        cursor.setCharFormat(format);
        
        block = block.next();
    }
    
    cursor.endEditBlock();
}

// NEW: Get text color based on content and current theme
QColor OutputWindow::getTextColor(const QString& text) const
{
    bool isLightTheme = (currentTheme == Settings::Light);
    bool isHackerTheme = (currentTheme == Settings::Hacker);
    
    if (text.contains("error:", Qt::CaseInsensitive) || 
        text.contains("fatal", Qt::CaseInsensitive)) {
        if (isHackerTheme) return QColor(255, 50, 50);
        return isLightTheme ? QColor(200, 0, 0) : QColor(255, 100, 100);
    }
    else if (text.contains("warning:", Qt::CaseInsensitive)) {
        if (isHackerTheme) return QColor(255, 255, 0);
        return isLightTheme ? QColor(180, 120, 0) : QColor(255, 200, 100);
    }
    else if (text.startsWith(">>")) {
        if (isHackerTheme) return QColor(0, 255, 255);
        return isLightTheme ? QColor(0, 100, 180) : QColor(100, 200, 255);
    }
    else {
        if (isHackerTheme) return QColor(0, 255, 0);
        return isLightTheme ? QColor(0, 0, 0) : QColor(212, 212, 212);
    }
}

void OutputWindow::appendCompileOutput(const QString& text)
{
    QTextCursor cursor = compileOutput->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(getTextColor(text));
    
    if (text.startsWith(">>")) {
        format.setFontWeight(QFont::Bold);
    }
    
    cursor.setCharFormat(format);
    cursor.insertText(text);
    compileOutput->setTextCursor(cursor);
    
    QScrollBar* scrollBar = compileOutput->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void OutputWindow::appendCompileError(const QString& text)
{
    appendCompileOutput(text);
}

void OutputWindow::appendRunOutput(const QString& text)
{
    QTextCursor cursor = runOutput->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(getTextColor(text));
    
    if (text.startsWith(">>")) {
        format.setFontWeight(QFont::Bold);
    }
    
    cursor.setCharFormat(format);
    cursor.insertText(text);
    runOutput->setTextCursor(cursor);
    
    QScrollBar* scrollBar = runOutput->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void OutputWindow::appendRunError(const QString& text)
{
    QTextCursor cursor = runOutput->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    QTextCharFormat errorFormat;
    errorFormat.setForeground(getTextColor("error:"));  // Use error color
    cursor.setCharFormat(errorFormat);
    cursor.insertText(text);
    runOutput->setTextCursor(cursor);
    
    QScrollBar* scrollBar = runOutput->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void OutputWindow::clearCompileOutput()
{
    compileOutput->clear();
}

void OutputWindow::clearRunOutput()
{
    runOutput->clear();
}

void OutputWindow::clearAll()
{
    compileOutput->clear();
    runOutput->clear();
}

void OutputWindow::showCompileTab()
{
    tabWidget->setCurrentIndex(0);
}

void OutputWindow::showRunTab()
{
    tabWidget->setCurrentIndex(1);
}