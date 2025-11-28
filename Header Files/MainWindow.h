#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QLabel>
#include <QMap>

#include "TextEditor.h"
#include "CppSyntaxHighlighter.h"
#include "SpellChecker.h"
#include "AIAssistant.h"
#include "Settings.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    // UI
    QTabWidget* tabWidget;              // Onglets pour les fichiers
    QLabel* statusLabel;

    // Gestion multi-documents
    TextEditor* textEditor;             // ← Gère PLUSIEURS documents
    QMap<int, QPlainTextEdit*> textAreas;        // Zone de texte par onglet
    QMap<int, CppSyntaxHighlighter*> highlighters; // Highlighter par onglet

    // Autres composants
    SpellChecker* spellChecker;
    AIAssistant* aiAssistant;
    Settings* settings;

    int zoomLevel;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void createUI();
    void createMenus();
    void connectSignals();

private slots:
    // Actions fichiers
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onFileSaveAll();
    void onFileClose();
    void onFileCloseAll();

    // Onglets
    void onTabChanged(int index);
    void onTabCloseRequested(int index);

    // Signaux de TextEditor
    void onDocumentOpened(int index, QString title);
    void onDocumentClosed(int index);
    void onDocumentModified(int index, bool modified);
    void onCurrentDocumentChanged(int index);

    // Autres
    void onTextChanged();
    void onThemeDark();
    void onThemeLight();
};

#endif