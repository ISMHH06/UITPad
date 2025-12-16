#include "MainWindow.h"
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"
#include "HybridHighlighter.h"

#include <QMenuBar>
#include <QToolBar>  // Pour la barre d'outils
#include <QStyle>    // Pour les icônes standard
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequence>
#include <QVBoxLayout>
#include <QDebug>
#include <QTextCursor>
#include <QInputDialog>
#include <QFileInfo>
#include <QStatusBar>

// --- Constructeur ---
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    spellChecker = new SpellChecker(":/dictionary.txt");
    textEditor = new TextEditor(this);

    // Initialisation des états par défaut
    isSyntaxHighlightingEnabled = true;
    isSpellCheckEnabled = true;

    // IMPORTANT : Initialiser le thème par défaut pour éviter le bug
    currentTheme = Settings::Light;

    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);

    // --- Connexions TabWidget ---
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    // --- Connexions TextEditor ---
    connect(textEditor, &TextEditor::documentOpened, this, &MainWindow::onDocumentOpened);
    connect(textEditor, &TextEditor::documentClosed, this, &MainWindow::onDocumentClosed);
    connect(textEditor, &TextEditor::documentSaved, this, &MainWindow::onDocumentSaved);
    connect(textEditor, &TextEditor::currentDocumentChanged, this, &MainWindow::onCurrentDocumentChanged);

    // --- Menu Fichier ---
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

    // --- Menu Édition ---
    QMenu* editMenu = menuBar()->addMenu("Édition");

    // On stocke les actions Undo/Redo pour pouvoir les griser si nécessaire
    actionUndo = editMenu->addAction("Annuler", QKeySequence::Undo, this, &MainWindow::onUndo);
    actionRedo = editMenu->addAction("Rétablir", QKeySequence::Redo, this, &MainWindow::onRedo);

    editMenu->addSeparator();
    editMenu->addAction("Aller à la ligne...", QKeySequence("Ctrl+G"), this, &MainWindow::onGoToLine);
    editMenu->addSeparator();
    editMenu->addAction("Paramètres...", this, &MainWindow::onOpenSettings);

    // --- Menu Affichage ---
    QMenu* viewMenu = menuBar()->addMenu("Affichage");
    viewMenu->addAction("Zoom Avant", QKeySequence::ZoomIn, this, &MainWindow::onZoomIn);
    viewMenu->addAction("Zoom Arrière", QKeySequence::ZoomOut, this, &MainWindow::onZoomOut);
    viewMenu->addSeparator();

    QAction* syntaxAction = viewMenu->addAction("Coloration syntaxique C++", this, &MainWindow::onToggleSyntaxHighlighting);
    syntaxAction->setCheckable(true);
    syntaxAction->setChecked(isSyntaxHighlightingEnabled);

    QAction* spellAction = viewMenu->addAction("Correction orthographique", this, &MainWindow::onToggleSpellCheck);
    spellAction->setCheckable(true);
    spellAction->setChecked(isSpellCheckEnabled);

    // --- Initialisation Toolbar et Status Bar ---
    setupToolbar();

    // Création du label pour "Ligne : X, Col : Y"
    statusLabel = new QLabel("Ligne : 1, Col : 1", this);
    statusLabel->setStyleSheet("padding-right: 15px;");
    statusBar()->addPermanentWidget(statusLabel);

    setWindowTitle("UITPad - Sans titre");
    resize(900, 650);
}

MainWindow::~MainWindow() {
    delete spellChecker;
}

// --- NOUVEAU : Configuration de la barre d'outils ---
void MainWindow::setupToolbar() {
    QToolBar* toolbar = addToolBar("Barre d'outils principale");
    toolbar->setMovable(false); // Figer la barre

    // Utilisation des icônes standard de Qt
    toolbar->addAction(style()->standardIcon(QStyle::SP_FileIcon), "Nouveau", this, &MainWindow::onFileNew);
    toolbar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Ouvrir", this, &MainWindow::onFileOpen);
    toolbar->addAction(style()->standardIcon(QStyle::SP_DriveFDIcon), "Sauvegarder", this, &MainWindow::onFileSave);

    toolbar->addSeparator();

    toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), "Annuler", this, &MainWindow::onUndo);
    toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), "Rétablir", this, &MainWindow::onRedo);

    toolbar->addSeparator();

    // Boutons de Zoom
    toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowUp), "Zoom In", this, &MainWindow::onZoomIn);
    toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowDown), "Zoom Out", this, &MainWindow::onZoomOut);
}

// --- Fichier ---

void MainWindow::onFileNew() { textEditor->createNewDocument(); }

void MainWindow::onFileOpen() {
    QString filename = QFileDialog::getOpenFileName(this, "Ouvrir un fichier");
    if (!filename.isEmpty()) {
        if (!textEditor->openDocument(filename))
            QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier.");
    }
}

void MainWindow::onFileRename() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;
    if (!doc->hasFilePath()) { onFileSaveAs(); return; }

    QString oldPath = doc->getFilePath();
    QFileInfo fileInfo(oldPath);
    QString oldName = fileInfo.fileName();
    bool ok;
    QString newName = QInputDialog::getText(this, "Renommer", "Nouveau nom :", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        QString newPath = fileInfo.absolutePath() + "/" + newName;
        QFile file(oldPath);
        if (file.rename(newPath)) {
            doc->setFilePath(newPath);
            updateTabTitle(doc);
            updateWindowTitle();
            QMessageBox::information(this, "Succès", "Fichier renommé.");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de renommer.");
        }
    }
}

void MainWindow::onFileSave() {
    Document* doc = textEditor->getCurrentDocument();
    if (doc) {
        if (!doc->hasFilePath()) onFileSaveAs();
        else textEditor->saveDocument(doc);
    }
}

void MainWindow::onFileSaveAs() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;
    QString filename = QFileDialog::getSaveFileName(this, "Sauvegarder sous...");
    if (!filename.isEmpty()) {
        doc->setFilePath(filename);
        textEditor->saveDocument(doc);
    }
}

void MainWindow::onFileClose() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;
    if (doc->getIsModified()) {
        auto reply = QMessageBox::question(this, "Modifié", "Sauvegarder ?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (reply == QMessageBox::Save) onFileSave();
        else if (reply == QMessageBox::Cancel) return;
    }
    textEditor->closeDocument(doc);
}

// --- Fonctionnalités d'édition (Undo, Redo, Zoom, Goto) ---

void MainWindow::onUndo() {
    QPlainTextEdit* editor = getCurrentTextEdit();
    if (editor) editor->undo();
}

void MainWindow::onRedo() {
    QPlainTextEdit* editor = getCurrentTextEdit();
    if (editor) editor->redo();
}

void MainWindow::onZoomIn() {
    QPlainTextEdit* editor = getCurrentTextEdit();
    if (editor) editor->zoomIn(2);
}

void MainWindow::onZoomOut() {
    QPlainTextEdit* editor = getCurrentTextEdit();
    if (editor) editor->zoomOut(2);
}

void MainWindow::onGoToLine() {
    QPlainTextEdit* editor = getCurrentTextEdit();
    if (!editor) return;

    bool ok;
    int maxLine = editor->blockCount();
    int line = QInputDialog::getInt(this, "Aller à la ligne",
                                    QString("Numéro de ligne (1-%1) :").arg(maxLine),
                                    1, 1, maxLine, 1, &ok);
    if (ok) {
        QTextCursor cursor = editor->textCursor();
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
        editor->setTextCursor(cursor);
        editor->centerCursor();
    }
}

void MainWindow::onCursorPositionChanged() {
    QPlainTextEdit* editor = getCurrentTextEdit();
    if (editor) {
        QTextCursor cursor = editor->textCursor();
        int line = cursor.blockNumber() + 1;
        int col = cursor.columnNumber() + 1;
        statusLabel->setText(QString("Ligne : %1, Col : %2").arg(line).arg(col));

        updateUndoRedoState();
    }
}

void MainWindow::updateUndoRedoState() {
    QPlainTextEdit* editor = getCurrentTextEdit();
    if (editor) {
        actionUndo->setEnabled(editor->document()->isUndoAvailable());
        actionRedo->setEnabled(editor->document()->isRedoAvailable());
    } else {
        actionUndo->setEnabled(false);
        actionRedo->setEnabled(false);
    }
}

// --- Ouverture Document ---
void MainWindow::onDocumentOpened(Document* doc) {
    if (doc) {
        QPlainTextEdit* textEdit = new QPlainTextEdit(this);
        textEdit->setPlainText(doc->getContent());
        textEdit->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(textEdit, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::showContextMenu);

        Settings s(this);
        textEdit->setFont(s.getEditorFont());

        int index = tabWidget->addTab(textEdit, doc->getFileName());
        tabWidget->setCurrentIndex(index);
        textEdit->setProperty("document", QVariant::fromValue(doc));

        // Connexions pour l'interface (Status bar, Undo/Redo)
        connect(textEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::onCursorPositionChanged);
        connect(textEdit, &QPlainTextEdit::undoAvailable, this, &MainWindow::updateUndoRedoState);
        connect(textEdit, &QPlainTextEdit::redoAvailable, this, &MainWindow::updateUndoRedoState);

        applyHybridHighlighter(textEdit);

        connect(textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);

        updateTabTitle(doc);
        updateWindowTitle();

        // Appliquer le thème actuel au nouvel onglet
        Settings dummySettings(this);
        dummySettings.setCurrentTheme(currentTheme);
        onCursorPositionChanged();
    }
}

// --- Gestion des surligneurs (Highlighters) ---

void MainWindow::applyHybridHighlighter(QPlainTextEdit* textEdit) {
    if (!textEdit) return;
    HybridHighlighter* highlighter = new HybridHighlighter(textEdit->document(), spellChecker);
    highlighter->setSyntaxHighlightingEnabled(isSyntaxHighlightingEnabled);
    highlighter->setSpellCheckEnabled(isSpellCheckEnabled);
    textEdit->setProperty("highlighter", QVariant::fromValue(static_cast<QObject*>(highlighter)));
}

HybridHighlighter* MainWindow::getHighlighterForTextEdit(QPlainTextEdit* textEdit) const {
    if (!textEdit) return nullptr;
    QObject* obj = textEdit->property("highlighter").value<QObject*>();
    return qobject_cast<HybridHighlighter*>(obj);
}

void MainWindow::updateHighlighterSettings(QPlainTextEdit* textEdit) {
    HybridHighlighter* highlighter = getHighlighterForTextEdit(textEdit);
    if (highlighter) {
        highlighter->setSyntaxHighlightingEnabled(isSyntaxHighlightingEnabled);
        highlighter->setSpellCheckEnabled(isSpellCheckEnabled);
    }
}

void MainWindow::onToggleSyntaxHighlighting() {
    isSyntaxHighlightingEnabled = !isSyntaxHighlightingEnabled;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) updateHighlighterSettings(textEdit);
    }
    statusBar()->showMessage(isSyntaxHighlightingEnabled ? "Syntaxe : ON" : "Syntaxe : OFF", 2000);
}

void MainWindow::onToggleSpellCheck() {
    isSpellCheckEnabled = !isSpellCheckEnabled;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) updateHighlighterSettings(textEdit);
    }
    statusBar()->showMessage(isSpellCheckEnabled ? "Ortho : ON" : "Ortho : OFF", 2000);
}

void MainWindow::showContextMenu(const QPoint& pos) {
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    if (!textEdit) return;

    QMenu* menu = textEdit->createStandardContextMenu();

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
            } else {
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

// --- Paramètres (Settings) ---

void MainWindow::onOpenSettings() {
    Settings settingsDialog(this);

    // CORRECTION : On envoie le thème actuel à la fenêtre
    settingsDialog.setCurrentTheme(currentTheme);

    connect(&settingsDialog, &Settings::settingsChanged, this, [&]() {
        applySettings(&settingsDialog);
    });
    settingsDialog.exec();
}

void MainWindow::applySettings(Settings* s) {
    if (!s) return;

    QFont font = s->getEditorFont();
    QColor textColor = s->getEditorColor();

    // CORRECTION : On sauvegarde le thème choisi
    currentTheme = s->getSelectedTheme();
    Settings::AppTheme theme = currentTheme;

    QString backgroundColor;
    QString globalStyle;

    switch (theme) {
    case Settings::Dark:
        backgroundColor = "#1e1e1e";
        if (textColor == Qt::black) textColor = QColor("#d4d4d4");

        globalStyle = R"(
            QMainWindow { background-color: #2b2b2b; color: #ffffff; }
            QTabWidget::pane { border: 1px solid #444444; }
            QTabBar::tab { background: #3c3c3c; color: #ffffff; padding: 5px; }
            QTabBar::tab:selected { background: #1e1e1e; font-weight: bold; }
            QMenuBar { background-color: #2b2b2b; color: #ffffff; }
            QMenuBar::item:selected { background-color: #3c3c3c; }
            QMenu { background-color: #2b2b2b; color: #ffffff; }
            QMenu::item:selected { background-color: #3d3d3d; }
            QToolBar { background-color: #2b2b2b; border: none; }
            QToolButton { color: white; }
            QToolButton:hover { background-color: #3c3c3c; }
            QStatusBar { color: white; }
        )";
        break;

    case Settings::Hacker:
        backgroundColor = "#000000";
        if (textColor == Qt::black) textColor = QColor("#00ff00");

        globalStyle = R"(
            QMainWindow { background-color: #0f0f0f; color: #00ff00; }
            QTabWidget::pane { border: 1px solid #00ff00; }
            QTabBar::tab { background: #000000; color: #008800; padding: 5px; }
            QTabBar::tab:selected { background: #000000; color: #00ff00; border: 1px solid #00ff00; border-bottom: none; }
            QMenuBar { background-color: #000000; color: #00ff00; }
            QMenuBar::item:selected { background-color: #003300; }
            QMenu { background-color: #000000; color: #00ff00; }
            QMenu::item:selected { background-color: #003300; }
            QToolBar { background-color: #000000; border-bottom: 1px solid #00ff00; }
            QToolButton { color: #00ff00; }
            QToolButton:hover { background-color: #003300; }
            QStatusBar { color: #00ff00; }
        )";
        break;

    case Settings::Light:
    default:
        backgroundColor = "white";
        globalStyle = "";
        break;
    }

    this->setStyleSheet(globalStyle);

    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            textEdit->setFont(font);

            QString selectionColor = (theme == Settings::Hacker) ? "#00aa00" : "#264f78";
            QString selectionTextColor = "white";

            QString editorStyle = QString("QPlainTextEdit { color: %1; background-color: %2; "
                                          "selection-background-color: %3; selection-color: %4; }")
                                      .arg(textColor.name())
                                      .arg(backgroundColor)
                                      .arg(selectionColor)
                                      .arg(selectionTextColor);

            textEdit->setStyleSheet(editorStyle);
        }
    }
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
        // Rafraichir l'état (Barre d'état, boutons Undo/Redo) quand on change d'onglet
        onCursorPositionChanged();
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

            // Mise à jour de la status bar
            onCursorPositionChanged();
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
