#include "MainWindow.h"
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"
#include "HybridHighlighter.h"

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

// --- Constructeur ---
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    spellChecker = new SpellChecker(":/dictionary.txt");
    textEditor = new TextEditor(this);

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

void MainWindow::applySettings(Settings* s) {
    if (!s) return;

    QFont font = s->getEditorFont();
    QColor color = s->getEditorColor();
    Settings::AppTheme theme = s->getSelectedTheme();

    // Si mode Système, détecter le thème de Windows
    if (theme == Settings::System) {
        bool isDark = Settings::isSystemDarkMode();
        theme = isDark ? Settings::Dark : Settings::Light;
    }

    // NOUVEAU : Appliquer le thème à toute l'application
    applyThemeToApplication(theme);

    // Appliquer le thème à tous les onglets
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            textEdit->setFont(font);

            // Appliquer le style selon le thème
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
                // Mode Clair (Light)
                styleSheet = QString("QPlainTextEdit { "
                    "color: %1; "
                    "background-color: white; "
                    "selection-background-color: #0078d7; "
                    "selection-color: white; "
                    "}").arg(color.name());
            }

            textEdit->setStyleSheet(styleSheet);

            // Mettre à jour le highlighter avec le thème
            HybridHighlighter* highlighter = getHighlighterForTextEdit(textEdit);
            if (highlighter) {
                bool isDark = (theme == Settings::Dark || theme == Settings::Hacker);
                highlighter->setTheme(isDark);
            }
        }
    }
    // NOUVEAU : Sauvegarder
    currentTheme = theme;

    // Appliquer partout
    applyThemeToApplication(theme);
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

    // Détecter si système
    if (theme == Settings::System) {
        bool isDark = Settings::isSystemDarkMode();
        theme = isDark ? Settings::Dark : Settings::Light;
    }

    // NOUVEAU : Sauvegarder
    currentTheme = theme;

    // Appliquer
    applyThemeToApplication(theme);
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