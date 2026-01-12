#include "MainWindow.h"
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"
#include "HybridHighlighter.h"
#include "AIAssistant.h"
#include "AIAssistantDock.h"

#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequence>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QTextCursor>
#include <QInputDialog>
#include <QFileInfo>
#include <QDir>
#include <QStatusBar>
#include <QToolButton>
#include <QEvent>

// --- Constructeur ---
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    spellChecker = new SpellChecker(":/dictionary.txt");
    textEditor = new TextEditor(this);
    aiAssistant = new AIAssistant(this);

    // In MainWindow::MainWindow constructor, after creating aiAssistant:

    // Initialize compiler manager
    compilerManager = new CompilerManager(this);

    // Initialize output window
    outputWindow = new OutputWindow(this);
    addDockWidget(Qt::BottomDockWidgetArea, outputWindow);
    outputWindow->hide();

    // Connect compiler signals
    connect(compilerManager, &CompilerManager::compilationStarted,
        this, [this](const QString&) {
            outputWindow->clearCompileOutput();
            outputWindow->show();
            outputWindow->showCompileTab();
        });

    connect(compilerManager, &CompilerManager::compilationOutput,
        outputWindow, &OutputWindow::appendCompileOutput);

    connect(compilerManager, &CompilerManager::compilationError,
        outputWindow, &OutputWindow::appendCompileError);

    connect(compilerManager, &CompilerManager::compilationFinished,
        this, [this](bool success, int) {
            if (success) {
                statusBar()->showMessage("Compilation successful", 3000);
            }
            else {
                statusBar()->showMessage("Compilation failed", 3000);
            }
        });

    connect(compilerManager, &CompilerManager::executionStarted,
        this, [this](const QString&) {
            outputWindow->clearRunOutput();
            outputWindow->show();
            outputWindow->showRunTab();
        });

    connect(compilerManager, &CompilerManager::executionOutput,
        outputWindow, &OutputWindow::appendRunOutput);

    connect(compilerManager, &CompilerManager::executionError,
        outputWindow, &OutputWindow::appendRunError);

    connect(compilerManager, &CompilerManager::executionFinished,
        this, [this](int exitCode) {
            statusBar()->showMessage(
                QString("Program finished with exit code %1").arg(exitCode), 3000);
        });

    connect(outputWindow, &OutputWindow::stopCompilationRequested,
        compilerManager, &CompilerManager::stopCompilation);

    connect(outputWindow, &OutputWindow::stopExecutionRequested,
        compilerManager, &CompilerManager::stopExecution);

    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);

    connect(tabWidget, &QTabWidget::currentChanged,
        this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested,
        this, &MainWindow::onTabCloseRequested);

    connect(textEditor, &TextEditor::documentOpened,
        this, &MainWindow::onDocumentOpened);
    connect(textEditor, &TextEditor::documentClosed,
        this, &MainWindow::onDocumentClosed);
    connect(textEditor, &TextEditor::documentSaved,
        this, &MainWindow::onDocumentSaved);
    connect(textEditor, &TextEditor::currentDocumentChanged,
        this, &MainWindow::onCurrentDocumentChanged);

    // Menu Fichier
    QMenu* fileMenu = menuBar()->addMenu("Fichier");
    fileMenu->addAction("Nouveau", QKeySequence::New, this, &MainWindow::onFileNew);
    fileMenu->addAction("Ouvrir", QKeySequence::Open, this, &MainWindow::onFileOpen);
    fileMenu->addAction("Renommer...", this, &MainWindow::onFileRename);
    fileMenu->addSeparator();
    fileMenu->addAction("Sauvegarder", QKeySequence::Save, this, &MainWindow::onFileSave);
    fileMenu->addAction("Sauvegarder sous...", QKeySequence::SaveAs, this, &MainWindow::onFileSaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Fermer", QKeySequence::Close, this, &MainWindow::onFileClose);
    fileMenu->addAction("Quitter", QKeySequence::Quit, this, &QWidget::close);

    // Menu Édition
    QMenu* editMenu = menuBar()->addMenu("Édition");
    editMenu->addAction("Paramètres...", this, &MainWindow::onOpenSettings);

    // Menu Affichage - Options indépendantes
    QMenu* viewMenu = menuBar()->addMenu("Affichage");

    // Add Run menu
    QMenu* runMenu = menuBar()->addMenu("Run");
    runMenu->addAction("Compile", QKeySequence("F7"), this, &MainWindow::onCompile);
    runMenu->addAction("Compile and Run", QKeySequence("Ctrl+F5"), this, &MainWindow::onCompileAndRun);
    runMenu->addAction("Run", QKeySequence("F5"), this, &MainWindow::onRun);
    runMenu->addSeparator();
    runMenu->addAction("Stop Compilation", QKeySequence("Shift+F7"), this, &MainWindow::onStopCompilation);
    runMenu->addAction("Stop Execution", QKeySequence("Shift+F5"), this, &MainWindow::onStopExecution);
    runMenu->addSeparator();
    runMenu->addAction("Compiler Settings...", this, &MainWindow::onCompilerSettings);

    QAction* syntaxAction = viewMenu->addAction("Coloration syntaxique C++",
        this, &MainWindow::onToggleSyntaxHighlighting);
    syntaxAction->setCheckable(true);
    syntaxAction->setChecked(isSyntaxHighlightingEnabled);
    syntaxAction->setToolTip("Active/désactive la coloration C++ (ligne par ligne)");

    QAction* spellAction = viewMenu->addAction("Correction orthographique",
        this, &MainWindow::onToggleSpellCheck);
    spellAction->setCheckable(true);
    spellAction->setChecked(isSpellCheckEnabled);
    spellAction->setToolTip("Active/désactive la correction orthographique sur les lignes de texte");

    setWindowTitle("UITPad - Sans titre");

    // Initialize currentTheme properly
    Settings s(this);
    currentTheme = s.getSelectedTheme();  // Get from settings

    // If system mode, detect actual system theme
    if (currentTheme == Settings::System) {
        bool isDark = Settings::isSystemDarkMode();
        currentTheme = isDark ? Settings::Dark : Settings::Light;
    }

    // NOUVEAU : Appliquer le thème système au démarrage
    applySystemTheme();

    resize(800, 600);
}

MainWindow::~MainWindow() {
    delete spellChecker;
}

void MainWindow::onFileNew() {
    textEditor->createNewDocument();
}

void MainWindow::onFileOpen() {
    QString filename = QFileDialog::getOpenFileName(this, "Ouvrir un fichier");
    if (filename.isEmpty()) return;

    Document* doc = textEditor->openDocument(filename);
    if (!doc) QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier.");
}

void MainWindow::onFileRename() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

    if (!doc->hasFilePath()) {
        onFileSaveAs();
        return;
    }

    QString oldPath = doc->getFilePath();
    QFileInfo fileInfo(oldPath);
    QString oldName = fileInfo.fileName();

    bool ok;
    QString newName = QInputDialog::getText(this, "Renommer le fichier",
        "Nouveau nom :", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        QString newPath = fileInfo.absolutePath() + "/" + newName;
        QFile file(oldPath);

        if (file.rename(newPath)) {
            doc->setFilePath(newPath);
            updateTabTitle(doc);
            updateWindowTitle();
            QMessageBox::information(this, "Succès", "Fichier renommé avec succès.");
        }
        else {
            QMessageBox::warning(this, "Erreur", "Impossible de renommer le fichier.");
        }
    }
}

void MainWindow::onFileSave() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

    if (!doc->hasFilePath()) {
        onFileSaveAs();
        return;
    }
    if (!textEditor->saveDocument(doc)) {
        QMessageBox::warning(this, "Erreur", "Impossible de sauvegarder.");
    }
}

void MainWindow::onFileSaveAs() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

    QString filename = QFileDialog::getSaveFileName(this, "Sauvegarder sous...");
    if (filename.isEmpty()) return;

    doc->setFilePath(filename);
    if (!textEditor->saveDocument(doc)) {
        QMessageBox::warning(this, "Erreur", "Impossible de sauvegarder.");
    }
}

void MainWindow::onFileClose() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

    if (doc->getIsModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Modifié",
            "Sauvegarder les modifications ?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) onFileSave();
        else if (reply == QMessageBox::Cancel) return;
    }
    textEditor->closeDocument(doc);
}

// --- Ouverture Document ---
void MainWindow::onDocumentOpened(Document* doc) {
    if (doc) {
        QPlainTextEdit* textEdit = new QPlainTextEdit(this);
        textEdit->setPlainText(doc->getContent());

        textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(textEdit, &QPlainTextEdit::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

        Settings s(this);
        textEdit->setFont(s.getEditorFont());

        int index = tabWidget->addTab(textEdit, doc->getFileName());
        tabWidget->setCurrentIndex(index);
        textEdit->setProperty("document", QVariant::fromValue(doc));

        // Appliquer le highlighter hybride
        applyHybridHighlighter(textEdit);

        // AI assistant overlay button (shows when selection is non-empty)
        setupAiForTextEdit(textEdit);

        connect(textEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::onTextChanged);

        updateTabTitle(doc);
        updateWindowTitle();

        // NOUVEAU : Appliquer le thème actuel
        applyThemeToTextEdit(textEdit, currentTheme);  // Use currentTheme

        // NOUVEAU : Set theme for highlighter too
        HybridHighlighter* highlighter = getHighlighterForTextEdit(textEdit);
        if (highlighter) {
            bool isDark = (currentTheme == Settings::Dark || currentTheme == Settings::Hacker);
            highlighter->setTheme(isDark);
        }
    }
}

// --- NOUVEAU : Appliquer le highlighter hybride ---
void MainWindow::applyHybridHighlighter(QPlainTextEdit* textEdit) {
    if (!textEdit) return;

    // Créer un highlighter hybride qui gère tout
    HybridHighlighter* highlighter = new HybridHighlighter(textEdit->document(), spellChecker);

    // Configurer selon les préférences actuelles
    highlighter->setSyntaxHighlightingEnabled(isSyntaxHighlightingEnabled);
    highlighter->setSpellCheckEnabled(isSpellCheckEnabled);

    // NOUVEAU : Si c'est un fichier code (.cpp/.h), forcer le mode code
    Document* doc = textEdit->property("document").value<Document*>();
    if (doc && isCodeFile(doc)) {
        highlighter->setForceCodeMode(true);
    }

    // Sauvegarder pour référence future
    textEdit->setProperty("highlighter", QVariant::fromValue(static_cast<QObject*>(highlighter)));
}

// NOUVEAU : Vérifier si c'est un fichier code
bool MainWindow::isCodeFile(Document* doc) const {
    if (!doc || !doc->hasFilePath()) return false;

    QString filePath = doc->getFilePath();
    return filePath.endsWith(".cpp") ||
        filePath.endsWith(".h") ||
        filePath.endsWith(".hpp") ||
        filePath.endsWith(".c") ||
        filePath.endsWith(".cc") ||
        filePath.endsWith(".cxx") ||
        filePath.endsWith(".hxx") ||
        filePath.endsWith(".inl");
}

// --- NOUVEAU : Obtenir le highlighter d'un textEdit ---
HybridHighlighter* MainWindow::getHighlighterForTextEdit(QPlainTextEdit* textEdit) const {
    if (!textEdit) return nullptr;

    QObject* obj = textEdit->property("highlighter").value<QObject*>();
    return qobject_cast<HybridHighlighter*>(obj);
}

// --- NOUVEAU : Mettre à jour les paramètres du highlighter ---
void MainWindow::updateHighlighterSettings(QPlainTextEdit* textEdit) {
    HybridHighlighter* highlighter = getHighlighterForTextEdit(textEdit);
    if (highlighter) {
        highlighter->setSyntaxHighlightingEnabled(isSyntaxHighlightingEnabled);
        highlighter->setSpellCheckEnabled(isSpellCheckEnabled);
    }
}

// --- NOUVEAU : Basculer la coloration syntaxique ---
void MainWindow::onToggleSyntaxHighlighting() {
    isSyntaxHighlightingEnabled = !isSyntaxHighlightingEnabled;

    // Mettre à jour tous les onglets
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            updateHighlighterSettings(textEdit);
        }
    }

    QString status = isSyntaxHighlightingEnabled
        ? "Coloration syntaxique activée (ligne par ligne)"
        : "Coloration syntaxique désactivée";
    statusBar()->showMessage(status, 3000);
}

// --- NOUVEAU : Basculer la correction orthographique ---
void MainWindow::onToggleSpellCheck() {
    isSpellCheckEnabled = !isSpellCheckEnabled;

    // Mettre à jour tous les onglets
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            updateHighlighterSettings(textEdit);
        }
    }

    QString status = isSpellCheckEnabled
        ? "Correction orthographique activée (lignes de texte)"
        : "Correction orthographique désactivée";
    statusBar()->showMessage(status, 3000);
}

// --- Clic Droit ---
void MainWindow::showContextMenu(const QPoint& pos) {
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    if (!textEdit) return;

    QMenu* menu = textEdit->createStandardContextMenu();

    // AI action (uses selection if any)
    menu->addSeparator();
    QAction* askAiAction = menu->addAction("Ask AI…");
    connect(askAiAction, &QAction::triggered, this, [this, textEdit]() {
        openAiDockForTextEdit(textEdit);
        });

    // Afficher les suggestions uniquement si le correcteur est actif
    if (isSpellCheckEnabled && spellChecker && spellChecker->isValid()) {
        QTextCursor cursor = textEdit->cursorForPosition(pos);
        cursor.select(QTextCursor::WordUnderCursor);
        QString word = cursor.selectedText();

        if (!word.isEmpty() && !spellChecker->check(word)) {
            menu->addSeparator();
            QAction* title = menu->addAction("Correction :");
            title->setEnabled(false);

            QStringList suggestions = spellChecker->suggest(word);

            if (suggestions.isEmpty()) {
                QAction* noSugg = menu->addAction("(Aucune suggestion)");
                noSugg->setEnabled(false);
            }
            else {
                for (const QString& sugg : suggestions) {
                    QAction* action = menu->addAction(sugg);
                    connect(action, &QAction::triggered, this, [textEdit, cursor, sugg]() mutable {
                        textEdit->setTextCursor(cursor);
                        cursor.beginEditBlock();
                        cursor.insertText(sugg);
                        cursor.endEditBlock();
                        });
                }
            }
        }
    }

    menu->exec(textEdit->mapToGlobal(pos));
    delete menu;
}

void MainWindow::onOpenSettings() {
    Settings settingsDialog(this);
    connect(&settingsDialog, &Settings::settingsChanged, this, [&]() {
        applySettings(&settingsDialog);
        });
    settingsDialog.exec();
}

// In applySettings() method, after applying theme to all text edits, add:

void MainWindow::applySettings(Settings* s) {
    if (!s) return;

    QFont font = s->getEditorFont();
    QColor color = s->getEditorColor();
    Settings::AppTheme theme = s->getSelectedTheme();

    if (theme == Settings::System) {
        bool isDark = Settings::isSystemDarkMode();
        theme = isDark ? Settings::Dark : Settings::Light;
    }

    applyThemeToApplication(theme);

    // Apply to all open text editors
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            textEdit->setFont(font);
            applyThemeToTextEdit(textEdit, theme);

            HybridHighlighter* highlighter = getHighlighterForTextEdit(textEdit);
            if (highlighter) {
                bool isDark = (theme == Settings::Dark || theme == Settings::Hacker);
                highlighter->setTheme(isDark);
            }
        }
    }

    currentTheme = theme;

    applyThemeToApplication(theme);

    // NEW: Apply theme to output window
    if (outputWindow) {
        outputWindow->applyTheme(theme);
    }
}

// NOUVEAU : Appliquer le thème à toute l'application
void MainWindow::applyThemeToApplication(Settings::AppTheme theme) {
    QString appStyleSheet;

    if (theme == Settings::Dark) {
        // Mode Sombre - Toute l'application
        appStyleSheet = R"(
            QMainWindow {
                background-color: #1E1E1E;
                color: #D4D4D4;
            }
            QMenuBar {
                background-color: #2D2D30;
                color: #D4D4D4;
                border-bottom: 1px solid #3E3E42;
            }
            QMenuBar::item:selected {
                background-color: #3E3E42;
            }
            QMenu {
                background-color: #2D2D30;
                color: #D4D4D4;
                border: 1px solid #3E3E42;
            }
            QMenu::item:selected {
                background-color: #094771;
            }
            QTabWidget::pane {
                border: 1px solid #3E3E42;
                background-color: #252526;
            }
            QTabBar::tab {
                background-color: #2D2D30;
                color: #D4D4D4;
                padding: 8px 20px;
                border: 1px solid #3E3E42;
            }
            QTabBar::tab:selected {
                background-color: #1E1E1E;
                border-bottom: 2px solid #007ACC;
            }
            QTabBar::tab:hover {
                background-color: #3E3E42;
            }
            QStatusBar {
                background-color: #007ACC;
                color: white;
            }
        )";
    }
    else if (theme == Settings::Hacker) {
        // Mode Hacker - Thème Terminal
        appStyleSheet = R"(
            QMainWindow {
                background-color: #000000;
                color: #00FF00;
            }
            QMenuBar {
                background-color: #001100;
                color: #00FF00;
                border-bottom: 1px solid #00FF00;
                font-family: 'Courier New', monospace;
            }
            QMenuBar::item:selected {
                background-color: #003300;
            }
            QMenu {
                background-color: #001100;
                color: #00FF00;
                border: 1px solid #00FF00;
                font-family: 'Courier New', monospace;
            }
            QMenu::item:selected {
                background-color: #003300;
            }
            QTabWidget::pane {
                border: 1px solid #00FF00;
                background-color: #000000;
            }
            QTabBar::tab {
                background-color: #001100;
                color: #00FF00;
                padding: 8px 20px;
                border: 1px solid #00FF00;
                font-family: 'Courier New', monospace;
            }
            QTabBar::tab:selected {
                background-color: #000000;
                border-bottom: 2px solid #00FF00;
            }
            QTabBar::tab:hover {
                background-color: #003300;
            }
            QStatusBar {
                background-color: #003300;
                color: #00FF00;
                font-family: 'Courier New', monospace;
            }
        )";
    }
    else {
        // Mode Clair - Thème par défaut
        appStyleSheet = R"(
            QMainWindow {
                background-color: #F3F3F3;
                color: #000000;
            }
            QMenuBar {
                background-color: #F0F0F0;
                color: #000000;
                border-bottom: 1px solid #CCCCCC;
            }
            QMenuBar::item:selected {
                background-color: #E0E0E0;
            }
            QMenu {
                background-color: #FFFFFF;
                color: #000000;
                border: 1px solid #CCCCCC;
            }
            QMenu::item:selected {
                background-color: #0078D7;
                color: white;
            }
            QTabWidget::pane {
                border: 1px solid #CCCCCC;
                background-color: #FFFFFF;
            }
            QTabBar::tab {
                background-color: #F0F0F0;
                color: #000000;
                padding: 8px 20px;
                border: 1px solid #CCCCCC;
            }
            QTabBar::tab:selected {
                background-color: #FFFFFF;
                border-bottom: 2px solid #0078D7;
            }
            QTabBar::tab:hover {
                background-color: #E0E0E0;
            }
            QStatusBar {
                background-color: #0078D7;
                color: white;
            }
        )";
    }

    // Appliquer le style à toute l'application
    this->setStyleSheet(appStyleSheet);
}

// NOUVEAU : Appliquer le thème système au démarrage
void MainWindow::applySystemTheme() {
    Settings s(this);
    Settings::AppTheme theme = s.getSelectedTheme();

    if (theme == Settings::System) {
        bool isDark = Settings::isSystemDarkMode();
        theme = isDark ? Settings::Dark : Settings::Light;
    }

    currentTheme = theme;

    applyThemeToApplication(theme);

    // NEW: Apply theme to output window
    if (outputWindow) {
        outputWindow->applyTheme(theme);
    }

    applySettings(&s);
}

void MainWindow::onDocumentClosed(Document* doc) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            Document* tabDoc = textEdit->property("document").value<Document*>();
            if (tabDoc == doc) {
                tabWidget->removeTab(i);
                textEdit->deleteLater();
                break;
            }
        }
    }
    updateWindowTitle();
}

void MainWindow::onDocumentSaved(Document* doc) {
    if (doc) updateTabTitle(doc);
    updateWindowTitle();
}

void MainWindow::onCurrentDocumentChanged(Document* doc) {
    if (doc) {
        for (int i = 0; i < tabWidget->count(); ++i) {
            QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
            if (textEdit) {
                Document* tabDoc = textEdit->property("document").value<Document*>();
                if (tabDoc == doc) {
                    tabWidget->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
    updateWindowTitle();
}

void MainWindow::onTextChanged() {
    QPlainTextEdit* textEdit = getCurrentTextEdit();
    if (textEdit) {
        Document* doc = textEdit->property("document").value<Document*>();
        if (doc) {
            doc->setContent(textEdit->toPlainText());
            updateTabTitle(doc);
            updateWindowTitle();
        }
    }
}

void MainWindow::onTabChanged(int index) {
    if (index >= 0 && index < tabWidget->count()) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
        if (textEdit) {
            Document* doc = textEdit->property("document").value<Document*>();
            if (doc) textEditor->switchToDocument(doc);
        }
    }
}

void MainWindow::onTabCloseRequested(int index) {
    if (index >= 0 && index < tabWidget->count()) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
        if (textEdit) {
            Document* doc = textEdit->property("document").value<Document*>();
            if (doc) onFileClose();
        }
    }
}

QPlainTextEdit* MainWindow::getCurrentTextEdit() const {
    int index = tabWidget->currentIndex();
    if (index >= 0 && index < tabWidget->count())
        return qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
    return nullptr;
}

QPlainTextEdit* MainWindow::getTextEditForDocument(Document* doc) const {
    if (!doc) return nullptr;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            Document* tabDoc = textEdit->property("document").value<Document*>();
            if (tabDoc == doc) return textEdit;
        }
    }
    return nullptr;
}

void MainWindow::updateTabTitle(Document* doc) {
    if (!doc) return;
    QPlainTextEdit* textEdit = getTextEditForDocument(doc);
    if (textEdit) {
        int index = tabWidget->indexOf(textEdit);
        if (index >= 0) {
            QString title = doc->getFileName();
            if (doc->getIsModified()) title = "*" + title;
            tabWidget->setTabText(index, title);
        }
    }
}

void MainWindow::updateWindowTitle() {
    Document* doc = textEditor->getCurrentDocument();
    if (doc) {
        QString title = doc->getFileName();
        if (doc->getIsModified()) title = "*" + title;
        title += " - UITPad";
        setWindowTitle(title);
    }
    else {
        setWindowTitle("UITPad - Sans titre");
    }
}

void MainWindow::applyThemeToTextEdit(
    QPlainTextEdit* textEdit,
    Settings::AppTheme theme
) {
    QString styleSheet;

    if (theme == Settings::Dark) {
        // Mode Sombre
        styleSheet = "QPlainTextEdit { "
            "color: #D4D4D4; "
            "background-color: #1E1E1E; "
            "selection-background-color: #264F78; "
            "selection-color: white; "
            "}";
    }
    else if (theme == Settings::Hacker) {
        // Mode Hacker (Terminal)
        styleSheet = "QPlainTextEdit { "
            "color: #00FF00; "
            "background-color: #000000; "
            "selection-background-color: #003300; "
            "selection-color: #00FF00; "
            "font-family: 'Courier New', monospace; "
            "}";
    }
    else {
        // Mode Clair (Light) - default
        Settings s(this);
        QColor color = s.getEditorColor();
        styleSheet = QString("QPlainTextEdit { "
            "color: %1; "
            "background-color: white; "
            "selection-background-color: #0078d7; "
            "selection-color: white; "
            "}").arg(color.name());
    }

    textEdit->setStyleSheet(styleSheet);
}

void MainWindow::setupAiForTextEdit(QPlainTextEdit* textEdit)
{
    if (!textEdit) return;

    QToolButton* btn = ensureAiButton(textEdit);
    if (btn) btn->hide();

    connect(textEdit, &QPlainTextEdit::selectionChanged, this, [this, textEdit]() {
        updateAiButtonForTextEdit(textEdit);
        updateAiDockSelectionFromTextEdit(textEdit);
        });
    connect(textEdit, &QPlainTextEdit::cursorPositionChanged, this, [this, textEdit]() {
        updateAiButtonForTextEdit(textEdit);
        updateAiDockSelectionFromTextEdit(textEdit);
        });

    textEdit->viewport()->installEventFilter(this);
}

QToolButton* MainWindow::ensureAiButton(QPlainTextEdit* textEdit)
{
    if (!textEdit) return nullptr;

    QVariant existing = textEdit->property("aiButton");
    if (existing.isValid()) {
        QToolButton* btn = qobject_cast<QToolButton*>(existing.value<QObject*>());
        if (btn) return btn;
    }

    QToolButton* btn = new QToolButton(textEdit->viewport());
    btn->setText("AI");
    btn->setToolTip("Ask AI about the selected text");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setAutoRaise(true);
    btn->setStyleSheet(
        "QToolButton {"
        "  background: #007ACC; color: white; border: 0px;"
        "  padding: 4px 8px; border-radius: 10px; font-weight: 600;"
        "}"
        "QToolButton:hover { background: #1493E6; }"
    );

    connect(btn, &QToolButton::clicked, this, [this, textEdit]() {
        openAiDockForTextEdit(textEdit);
        });

    textEdit->setProperty("aiButton", QVariant::fromValue(static_cast<QObject*>(btn)));
    return btn;
}

void MainWindow::updateAiButtonForTextEdit(QPlainTextEdit* textEdit)
{
    if (!textEdit) return;

    QToolButton* btn = ensureAiButton(textEdit);
    if (!btn) return;

    QTextCursor c = textEdit->textCursor();
    if (!c.hasSelection() || c.selectedText().trimmed().isEmpty()) {
        btn->hide();
        return;
    }

    QTextCursor endCursor = c;
    endCursor.setPosition(c.selectionEnd());
    QRect r = textEdit->cursorRect(endCursor);

    QPoint p = r.topRight() + QPoint(8, -btn->sizeHint().height() / 2);

    const QRect vp = textEdit->viewport()->rect();
    const QSize s = btn->sizeHint();

    int x = qBound(0, p.x(), vp.width() - s.width());
    int y = qBound(0, p.y(), vp.height() - s.height());

    btn->move(x, y);
    btn->show();
    btn->raise();
}

static QString normalizedQtSelection(QString s)
{
    // Qt returns U+2029 for line breaks in selectedText()
    s.replace(QChar(0x2029), '\n');
    return s;
}

AIAssistantDock* MainWindow::ensureAiDock()
{
    if (aiDock) return aiDock;

    aiDock = new AIAssistantDock(aiAssistant, this);
    addDockWidget(Qt::RightDockWidgetArea, aiDock);
    aiDock->hide();
    return aiDock;
}

void MainWindow::updateAiDockSelectionFromTextEdit(QPlainTextEdit* textEdit)
{
    if (!aiDock || !aiDock->isVisible()) return;
    if (!textEdit) return;

    QString selected = normalizedQtSelection(textEdit->textCursor().selectedText());
    aiDock->setSelectionText(selected);
}

void MainWindow::openAiDockForTextEdit(QPlainTextEdit* textEdit)
{
    if (!textEdit) return;

    AIAssistantDock* dock = ensureAiDock();

    QString selected = normalizedQtSelection(textEdit->textCursor().selectedText());
    dock->setSelectionText(selected);

    dock->show();
    dock->raise();
    dock->setFocus();
    dock->focusQuestion();
}

// Add these implementations before the closing brace of MainWindow.cpp

void MainWindow::onCompile()
{
    Document* doc = textEditor->getCurrentDocument();
    if (!doc || !doc->hasFilePath()) {
        QMessageBox::warning(this, "Compile",
            "Please save the file before compiling.");
        return;
    }

    // Auto-save before compiling
    if (doc->getIsModified()) {
        if (!textEditor->saveDocument(doc)) {
            QMessageBox::warning(this, "Compile",
                "Failed to save file before compilation.");
            return;
        }
    }

    QString filePath = doc->getFilePath();
    compilerManager->compileFile(filePath);
}

void MainWindow::onCompileAndRun()
{
    Document* doc = textEditor->getCurrentDocument();
    if (!doc || !doc->hasFilePath()) {
        QMessageBox::warning(this, "Compile and Run",
            "Please save the file before compiling.");
        return;
    }

    // Auto-save before compiling
    if (doc->getIsModified()) {
        if (!textEditor->saveDocument(doc)) {
            QMessageBox::warning(this, "Compile and Run",
                "Failed to save file before compilation.");
            return;
        }
    }

    QString filePath = doc->getFilePath();
    compilerManager->compileAndRun(filePath);
}

void MainWindow::onRun()
{
    Document* doc = textEditor->getCurrentDocument();
    if (!doc || !doc->hasFilePath()) {
        QMessageBox::warning(this, "Run",
            "Please save the file first.");
        return;
    }

    QString sourceFile = doc->getFilePath();
    QFileInfo fileInfo(sourceFile);
    QString baseName = fileInfo.completeBaseName();
    QString dir = fileInfo.absolutePath();

#ifdef Q_OS_WIN
    QString executable = QDir(dir).filePath(baseName + ".exe");
#else
    QString executable = QDir(dir).filePath(baseName);
#endif

    if (!QFile::exists(executable)) {
        QMessageBox::warning(this, "Run",
            "Executable not found. Please compile first.");
        return;
    }

    compilerManager->runExecutable(executable);
}

void MainWindow::onStopCompilation()
{
    compilerManager->stopCompilation();
}

void MainWindow::onStopExecution()
{
    compilerManager->stopExecution();
}

void MainWindow::onCompilerSettings()
{
    // TODO: Create a compiler settings dialog
    QMessageBox::information(this, "Compiler Settings",
        QString("Current compiler: %1\nPath: %2\n\nC++ Standard: C++17\nOptimization: O0")
        .arg(compilerManager->getCompilerType() == CompilerManager::GCC ? "GCC/G++" : "Clang")
        .arg(compilerManager->getCompilerPath()));
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Resize ||
        event->type() == QEvent::Wheel ||
        event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::KeyRelease) {
        QPlainTextEdit* te = qobject_cast<QPlainTextEdit*>(obj ? obj->parent() : nullptr);
        if (te) {
            updateAiButtonForTextEdit(te);
            updateAiDockSelectionFromTextEdit(te);
        }
    }
    return QMainWindow::eventFilter(obj, event);
}