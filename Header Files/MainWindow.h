#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "HybridHighlighter.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"

class AIAssistant;
class QToolButton;
class QEvent;
class AIAssistantDock;

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

private:
    QTabWidget* tabWidget;
    TextEditor* textEditor;
    SpellChecker* spellChecker;
    AIAssistant* aiAssistant = nullptr;
    AIAssistantDock* aiDock = nullptr;

    // Variables d'état
    bool isSyntaxHighlightingEnabled = true;  // Activé par défaut
    bool isSpellCheckEnabled = true;           // Activé par défaut
    Settings::AppTheme currentTheme = Settings::System;  // NOUVEAU : Stocker le thème actuel

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

    // NOUVEAU : Vérifier si c'est un fichier code (extension .cpp/.h/etc)
    bool isCodeFile(Document* doc) const;

    // NOUVEAU : Gestion des thèmes
    void applyThemeToApplication(Settings::AppTheme theme);
    void applyThemeToTextEdit(QPlainTextEdit* textEdit, Settings::AppTheme theme);
    void applySystemTheme();

    // AI assistant UI
    void setupAiForTextEdit(QPlainTextEdit* textEdit);
    void updateAiButtonForTextEdit(QPlainTextEdit* textEdit);
    QToolButton* ensureAiButton(QPlainTextEdit* textEdit);
    AIAssistantDock* ensureAiDock();
    void openAiDockForTextEdit(QPlainTextEdit* textEdit);
    void updateAiDockSelectionFromTextEdit(QPlainTextEdit* textEdit);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // MAINWINDOW_H