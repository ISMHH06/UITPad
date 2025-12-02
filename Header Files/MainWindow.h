#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTabWidget>

// Forward declarations
class TextEditor;
class Document;

// MainWindow: Fenêtre principale de l'application UITPad
// Gère l'interface utilisateur et coordonne TextEditor et Document
class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QTabWidget* tabWidget;         // Widget d'onglets pour les documents
    TextEditor* textEditor;        // Gestionnaire de documents (DO NOT REMOVE - required by MOC)

    QPlainTextEdit* getCurrentTextEdit() const;
    QPlainTextEdit* getTextEditForDocument(Document* doc) const;
    void updateWindowTitle();
    void updateTabTitle(Document* doc);
    void updateTextAreaContent();

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Menu Fichier
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onFileClose();
    
    // Signaux de TextEditor
    void onDocumentOpened(Document* doc);
    void onDocumentClosed(Document* doc);
    void onDocumentSaved(Document* doc);
    void onCurrentDocumentChanged(Document* doc);
    
    // Gestion des onglets
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    
    // Changements de texte
    void onTextChanged();
};

#endif