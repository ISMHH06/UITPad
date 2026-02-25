#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "HybridHighlighter.h"
#include "CompilerManager.h"
#include "OutputWindow.h"
#include "ThemeManager.h"
#include "CodeEditorWidget.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QDockWidget>
#include <QLabel>
#include <QListWidget>
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"
#include "AIAssistant.h"
#include "AISettingsDialog.h"
#include "IAChatWidget.h"

class QToolButton;
class QEvent;

// Filter proxy to hide build artifact directories in the project explorer
class ProjectFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit ProjectFilterProxy(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override {
        QFileSystemModel *fsModel = qobject_cast<QFileSystemModel*>(sourceModel());
  if (!fsModel) return true;

    QModelIndex idx = fsModel->index(sourceRow, 0, sourceParent);
        QString name = fsModel->fileName(idx);

        // Hide build artifact directories and hidden directories
        if (fsModel->isDir(idx)) {
            static const QStringList hiddenDirs = {
 ".qt", ".vs", ".git", ".vscode", ".idea",
        "CMakeFiles", "CMakeScripts",
           "out", "build", "debug", "release",
 "x64", "x86", "Win32",
      "UITPad_autogen",
        "generic", "iconengines", "imageformats",
      "networkinformation", "platforms", "styles",
              "Testing", "tls", "translations"
       };
            if (name.startsWith(".") || hiddenDirs.contains(name, Qt::CaseInsensitive))
        return false;
        } else {
      // Hide build artifacts / cache files
        static const QStringList hiddenFiles = {
       "cmake_install.cmake", "CMakeCache.txt",
    "VSInheritEnvironments.txt", "compile_commands.json"
  };
 if (hiddenFiles.contains(name, Qt::CaseInsensitive))
       return false;
        }

 return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }
};

class MainWindow : public QMainWindow {
 Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Gestion des fichiers
    void onFileNew();
    void onFileOpen();
    void onOpenFolder();
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

    // Project explorer
    void toggleProjectExplorer();
  void onProjectFileClicked(const QModelIndex &index);

    // Open editors list
    void onOpenEditorItemClicked(QListWidgetItem *item);
    void onOpenEditorCloseClicked(int tabIndex);
    void toggleOpenEditorsSection();
  void toggleFolderSection();

    // Status bar updates
    void updateCursorPosition();

private:
    QTabWidget* tabWidget;
    TextEditor* textEditor;
    SpellChecker* spellChecker;
    CompilerManager* compilerManager;
    OutputWindow* outputWindow;

    // Toolbar
    QToolBar* mainToolBar = nullptr;

    // Project explorer
    QDockWidget* projectDock = nullptr;
    QTreeView* projectTree = nullptr;
    QFileSystemModel* fileSystemModel = nullptr;
    ProjectFilterProxy* projectFilterProxy = nullptr;
    bool explorerHasFolder = false;  // Track if user has opened a folder

    // Open editors section
    QListWidget* openEditorsList = nullptr;
    QWidget* openEditorsContainer = nullptr;
 QWidget* openEditorsHeader = nullptr;
    QLabel* openEditorsArrow = nullptr;
    bool openEditorsSectionExpanded = true;

    // Folder section
    QWidget* folderContainer = nullptr;
    QWidget* folderHeader = nullptr;
    QLabel* folderArrow = nullptr;
  QLabel* folderTitleLabel = nullptr;
    QWidget* placeholderWidget = nullptr;
    bool folderSectionExpanded = true;

    // Status bar widgets
    QLabel* statusLineCol = nullptr;
    QLabel* statusEncoding = nullptr;
    QLabel* statusFileType = nullptr;
    QLabel* statusBuildState = nullptr;

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
    void updateProjectExplorerRoot(const QString &filePath);
    void syncOpenEditorsList();

// Setup methods
    void setupToolBar();
    void setupProjectExplorer();
    void setupStatusBar();
    void setupMenus();
    void createTabCloseIcon();

    // Gestion du highlighter hybride
    void applyHybridHighlighter(QPlainTextEdit* textEdit);
    void updateHighlighterSettings(QPlainTextEdit* textEdit);
    HybridHighlighter* getHighlighterForTextEdit(QPlainTextEdit* textEdit) const;
    bool isCodeFile(Document* doc) const;

    // Gestion des thèmes
    void applyThemeToApplication(Settings::AppTheme theme);
    void applyThemeToTextEdit(QPlainTextEdit* textEdit, Settings::AppTheme theme);
    void applySystemTheme();
    void applyThemeToAllComponents(Settings::AppTheme theme);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // MAINWINDOW_H