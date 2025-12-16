#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QLabel> // Pour la barre d'état
#include <QAction>
#include "Settings.h"

// Déclarations anticipées des classes
class SpellChecker;
class TextEditor;
class Document;
class HybridHighlighter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // --- Menu Fichier ---
    void onFileNew();
    void onFileOpen();
    void onFileRename();
    void onFileSave();
    void onFileSaveAs();
    void onFileClose();

    // --- Menu Edition & Affichage ---
    void onOpenSettings();
    void onToggleSyntaxHighlighting();
    void onToggleSpellCheck();

    // --- NOUVEAU : Fonctionnalités d'édition ---
    void onUndo();
    void onRedo();
    void onZoomIn();
    void onZoomOut();
    void onGoToLine(); // Aller à la ligne spécifique

    // --- NOUVEAU : Mise à jour de l'interface ---
    void onCursorPositionChanged(); // Met à jour Ligne/Col en bas
    void updateUndoRedoState();     // Active/Désactive les boutons grisés

    // --- Gestion des Onglets et Documents ---
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void onDocumentOpened(Document* doc);
    void onDocumentClosed(Document* doc);
    void onDocumentSaved(Document* doc);
    void onCurrentDocumentChanged(Document* doc);
    void onTextChanged();

    // Menu contextuel (clic droit)
    void showContextMenu(const QPoint& pos);

private:
    // --- Composants principaux ---
    QTabWidget* tabWidget;
    SpellChecker* spellChecker;
    TextEditor* textEditor;

    // --- NOUVEAU : Widget de la barre d'état ---
    QLabel* statusLabel;

    // --- Actions globales (pour pouvoir les activer/désactiver) ---
    QAction* actionUndo;
    QAction* actionRedo;

    // --- États ---
    bool isSyntaxHighlightingEnabled;
    bool isSpellCheckEnabled;

    // Variable pour retenir le thème actuel (Correction bug thème)
    Settings::AppTheme currentTheme;

    // --- Méthodes utilitaires ---
    void setupToolbar(); // Création de la barre d'outils
    void updateTabTitle(Document* doc);
    void updateWindowTitle();
    void applySettings(Settings* s);
    void applyHybridHighlighter(QPlainTextEdit* textEdit);
    void updateHighlighterSettings(QPlainTextEdit* textEdit);

    QPlainTextEdit* getCurrentTextEdit() const;
    QPlainTextEdit* getTextEditForDocument(Document* doc) const;
    HybridHighlighter* getHighlighterForTextEdit(QPlainTextEdit* textEdit) const;
};

#endif // MAINWINDOW_H
