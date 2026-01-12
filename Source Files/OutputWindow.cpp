#include "OutputWindow.h"
#include <QVBoxLayout>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QScrollBar>
#include <QTextBlock>

OutputWindow::OutputWindow(QWidget* parent)
    : QDockWidget("Output", parent)
{
    setupUI();
}

void OutputWindow::setupUI()
{
    QWidget* mainWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    tabWidget = new QTabWidget(mainWidget);

    // Compilation output tab
    QWidget* compileTab = new QWidget();
    QVBoxLayout* compileLayout = new QVBoxLayout(compileTab);
    compileLayout->setContentsMargins(4, 4, 4, 4);

    compileOutput = new QPlainTextEdit(compileTab);
    compileOutput->setReadOnly(true);
    compileLayout->addWidget(compileOutput);

    QHBoxLayout* compileButtons = new QHBoxLayout();
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
    runLayout->setContentsMargins(4, 4, 4, 4);

    runOutput = new QPlainTextEdit(runTab);
    runOutput->setReadOnly(true);
    runLayout->addWidget(runOutput);

    QHBoxLayout* runButtons = new QHBoxLayout();
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

    setMinimumHeight(150);
    
    applyTheme(Settings::Dark);
}

void OutputWindow::applyTheme(Settings::AppTheme theme)
{
    currentTheme = theme;
    
    QString outputStyle;
    QString widgetStyle;
    
    if (theme == Settings::Dark) {
        outputStyle = R"(
            QPlainTextEdit {
                background-color: #1E1E1E;
                color: #D4D4D4;
                font-family: 'Consolas', 'Courier New', monospace;
                font-size: 10pt;
                border: 1px solid #3E3E42;
            }
        )";
        widgetStyle = R"(
            QDockWidget {
                background-color: #252526;
                color: #D4D4D4;
            }
            QDockWidget::title {
                background-color: #2D2D30;
                color: #D4D4D4;
                padding: 6px;
            }
            QPushButton {
                background-color: #2D2D30;
                color: #D4D4D4;
                border: 1px solid #3E3E42;
                padding: 4px 12px;
                border-radius: 3px;
            }
            QPushButton:hover {
                background-color: #3E3E42;
            }
            QTabWidget::pane {
                border: 1px solid #3E3E42;
                background-color: #252526;
            }
            QTabBar::tab {
                background-color: #2D2D30;
                color: #D4D4D4;
                padding: 6px 20px;
                border: 1px solid #3E3E42;
            }
            QTabBar::tab:selected {
                background-color: #1E1E1E;
                border-bottom: 2px solid #007ACC;
            }
        )";
    }
    else if (theme == Settings::Hacker) {
        outputStyle = R"(
            QPlainTextEdit {
                background-color: #000000;
                color: #00FF00;
                font-family: 'Courier New', monospace;
                font-size: 10pt;
                border: 1px solid #00FF00;
            }
        )";
        widgetStyle = R"(
            QDockWidget {
                background-color: #000000;
                color: #00FF00;
            }
            QDockWidget::title {
                background-color: #001100;
                color: #00FF00;
                padding: 6px;
            }
            QPushButton {
                background-color: #001100;
                color: #00FF00;
                border: 1px solid #00FF00;
                padding: 4px 12px;
                border-radius: 3px;
                font-family: 'Courier New', monospace;
            }
            QPushButton:hover {
                background-color: #003300;
            }
            QTabWidget::pane {
                border: 1px solid #00FF00;
                background-color: #000000;
            }
            QTabBar::tab {
                background-color: #001100;
                color: #00FF00;
                padding: 6px 20px;
                border: 1px solid #00FF00;
                font-family: 'Courier New', monospace;
            }
            QTabBar::tab:selected {
                background-color: #000000;
                border-bottom: 2px solid #00FF00;
            }
        )";
    }
    else {
        // Light theme
        outputStyle = R"(
            QPlainTextEdit {
                background-color: #FFFFFF;
                color: #000000;
                font-family: 'Consolas', 'Courier New', monospace;
                font-size: 10pt;
                border: 1px solid #CCCCCC;
            }
        )";
        widgetStyle = R"(
            QDockWidget {
                background-color: #F3F3F3;
                color: #000000;
            }
            QDockWidget::title {
                background-color: #E0E0E0;
                color: #000000;
                padding: 6px;
            }
            QPushButton {
                background-color: #E0E0E0;
                color: #000000;
                border: 1px solid #CCCCCC;
                padding: 4px 12px;
                border-radius: 3px;
            }
            QPushButton:hover {
                background-color: #D0D0D0;
            }
            QTabWidget::pane {
                border: 1px solid #CCCCCC;
                background-color: #FFFFFF;
            }
            QTabBar::tab {
                background-color: #E0E0E0;
                color: #000000;
                padding: 6px 20px;
                border: 1px solid #CCCCCC;
            }
            QTabBar::tab:selected {
                background-color: #FFFFFF;
                border-bottom: 2px solid #0078D7;
            }
        )";
    }
    
    compileOutput->setStyleSheet(outputStyle);
    runOutput->setStyleSheet(outputStyle);
    widget()->setStyleSheet(widgetStyle);
    
    // IMPORTANT: Refresh existing text colors
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