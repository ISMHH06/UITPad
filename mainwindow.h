#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "highlighter.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include "texteditor.h"
#include "document.h"
#include "spellchecker.h"
#include "settings.h"  // <--- 1. IMPORTANT : On inclut les paramètres

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // --- Gestion des fichiers ---
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onFileClose();

    // --- Gestion des documents ---
    void onDocumentOpened(Document* doc);
    void onDocumentClosed(Document* doc);
    void onDocumentSaved(Document* doc);
    void onCurrentDocumentChanged(Document* doc);

    // --- Interface ---
    void onTextChanged();
    void onTabChanged(int index);
    void onTabCloseRequested(int index);

    void showContextMenu(const QPoint &pos);

    // --- 2. NOUVEAU : Slot pour ouvrir les paramètres ---
    void onOpenSettings();

private:
    QTabWidget* tabWidget;
    TextEditor* textEditor;
    SpellChecker* spellChecker;
    void onFileRename();

    // --- 3. NOUVEAU : Variable pour savoir si on corrige ou pas ---
    bool isCorrectionActive = true;

    // Méthodes internes
    void updateTabTitle(Document* doc);
    void updateWindowTitle();
    QPlainTextEdit* getCurrentTextEdit() const;
    QPlainTextEdit* getTextEditForDocument(Document* doc) const;

    // --- 4. NOUVEAU : Fonction pour appliquer police/couleur ---
    void applySettings(Settings* s);
};

#endif // MAINWINDOW_H
