#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "HybridHighlighter.h"
#include "CompilerManager.h"
#include "OutputWindow.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"
#include "AIAssistant.h"
#include "AISettingsDialog.h"
#include "IAChatWidget.h"

class QToolButton;
class QEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Gestion des fichiers
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onFileClose();
    void onFileRename();

    // Gestion des documents
    void onDocumentOpened(Document* doc);
    void onDocumentClosed(Document* doc);
    void onDocumentSaved(Document* doc);
    void onCurrentDocumentChanged(Document* doc);

    // Interface
    void onTextChanged();
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void showContextMenu(const QPoint& pos);

    // Paramètres et fonctionnalités
    void onOpenSettings();
    void onToggleSyntaxHighlighting();
    void onToggleSpellCheck();

    // Compiler slots
    void onCompile();
    void onCompileAndRun();
    void onRun();
    void onStopCompilation();
    void onStopExecution();
    void onCompilerSettings();

    // AI slots (NEW from teammate)
    void onExplainCode();
    void onCompleteCode();
    void onGenerateFromComment();
    void onAISettings();
    void onAIError(const QString &error);
    void toggleChatWidget();
    void onCodeInsertRequested(const QString &code);
    void onCodeCopyRequested(const QString &code);

private:
    QTabWidget* tabWidget;
    TextEditor* textEditor;
    SpellChecker* spellChecker;
    CompilerManager* compilerManager;
    OutputWindow* outputWindow;
    
    // AI components (NEW from teammate)
    AIAssistant* aiAssistant = nullptr;
    AISettingsDialog* aiSettingsDialog = nullptr;
    IAChatWidget* chatWidget = nullptr;

    // Variables d'état
    bool isSyntaxHighlightingEnabled = true;
    bool isSpellCheckEnabled = true;
    Settings::AppTheme currentTheme = Settings::System;

    // Méthodes internes
    void updateTabTitle(Document* doc);
    void updateWindowTitle();
    QPlainTextEdit* getCurrentTextEdit() const;
    QPlainTextEdit* getTextEditForDocument(Document* doc) const;
    void applySettings(Settings* s);

    // Gestion du highlighter hybride
    void applyHybridHighlighter(QPlainTextEdit* textEdit);
    void updateHighlighterSettings(QPlainTextEdit* textEdit);
    HybridHighlighter* getHighlighterForTextEdit(QPlainTextEdit* textEdit) const;
    bool isCodeFile(Document* doc) const;

    // Gestion des thèmes
    void applyThemeToApplication(Settings::AppTheme theme);
    void applyThemeToTextEdit(QPlainTextEdit* textEdit, Settings::AppTheme theme);
    void applySystemTheme();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // MAINWINDOW_H