#include "MainWindow.h"
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"
#include "highlighter.h"
#include "CppSyntaxHighlighter.h"
#include "CppSyntaxRules.h"  // Pour isCppCode()

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
#include <QTimer>
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

    // Menu Affichage - MODIFIÉ
    QMenu* viewMenu = menuBar()->addMenu("Affichage");
    QAction* syntaxAction = viewMenu->addAction("Détection automatique C++", this, &MainWindow::onToggleSyntaxHighlighting);
    syntaxAction->setCheckable(true);
    syntaxAction->setChecked(isSyntaxHighlightingEnabled);
    syntaxAction->setToolTip("Détecte et colore automatiquement le code C++ dans tous les fichiers");

    setWindowTitle("UITPad - Sans titre");
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
        "Nouveau nom :", QLineEdit::Normal,
        oldName, &ok);

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

// --- FONCTION CLÉE : Détection automatique de code C++ ---
bool MainWindow::shouldApplySyntaxHighlighting(QPlainTextEdit* textEdit) const {
    if (!textEdit || !isSyntaxHighlightingEnabled) {
        return false;
    }

    QString content = textEdit->toPlainText();

    // Si le texte est vide ou très court, pas de coloration
    if (content.trimmed().length() < 20) {
        return false;
    }

    // Utiliser la détection automatique
    return CppSyntaxRules::isCppCode(content);
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

        // Connexion pour détecter les changements de texte
        connect(textEdit, &QPlainTextEdit::textChanged, this, [this, textEdit]() {
            onTextChanged();
            // Analyser le contenu après un court délai pour éviter trop de recalculs
            checkAndApplyHighlighting(textEdit);
            });

        // Appliquer le highlighter initial
        applyAppropriateHighlighter(textEdit);

        updateTabTitle(doc);
        updateWindowTitle();
    }
}

// --- NOUVELLE FONCTION : Choisir le bon highlighter selon le contenu ---
void MainWindow::applyAppropriateHighlighter(QPlainTextEdit* textEdit) {
    if (!textEdit) return;

    // Supprimer l'ancien highlighter
    removeAllHighlighters(textEdit);

    // Déterminer quel highlighter appliquer
    if (shouldApplySyntaxHighlighting(textEdit)) {
        // Code C++ détecté → Coloration syntaxique
        CppSyntaxHighlighter* syntaxHighlighter = new CppSyntaxHighlighter(textEdit->document());
        textEdit->document()->setProperty("highlighter", QVariant::fromValue(static_cast<QObject*>(syntaxHighlighter)));
        textEdit->setProperty("highlighterType", "cpp");
    }
    else if (isCorrectionActive && spellChecker && spellChecker->isValid()) {
        // Texte normal → Correction orthographique
        Highlighter* spellHighlighter = new Highlighter(textEdit->document(), spellChecker);
        textEdit->document()->setProperty("highlighter", QVariant::fromValue(static_cast<QObject*>(spellHighlighter)));
        textEdit->setProperty("highlighterType", "spell");
    }
}

// --- NOUVELLE FONCTION : Vérifier et appliquer avec délai ---
void MainWindow::checkAndApplyHighlighting(QPlainTextEdit* textEdit) {
    if (!textEdit) return;

    // Utiliser un timer pour éviter de recalculer à chaque frappe
    static QTimer* timer = nullptr;
    if (!timer) {
        timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setInterval(500); // 500ms de délai
    }

    // Déconnecter l'ancien signal
    disconnect(timer, nullptr, nullptr, nullptr);

    // Connecter au nouvel onglet
    connect(timer, &QTimer::timeout, this, [this, textEdit]() {
        QString currentType = textEdit->property("highlighterType").toString();
        bool shouldBeCode = shouldApplySyntaxHighlighting(textEdit);

        // Changer uniquement si nécessaire
        if ((shouldBeCode && currentType != "cpp") || (!shouldBeCode && currentType == "cpp")) {
            applyAppropriateHighlighter(textEdit);
        }
        });

    timer->start();
}

// --- NOUVELLE FONCTION : Supprimer tous les highlighters ---
void MainWindow::removeAllHighlighters(QPlainTextEdit* textEdit) {
    if (!textEdit) return;

    QTextDocument* doc = textEdit->document();
    QSyntaxHighlighter* oldHighlighter = qobject_cast<QSyntaxHighlighter*>(
        doc->property("highlighter").value<QObject*>());

    if (oldHighlighter) {
        delete oldHighlighter;
        doc->setProperty("highlighter", QVariant());
        textEdit->setProperty("highlighterType", "");
    }
}

// --- Basculer la détection automatique ---
void MainWindow::onToggleSyntaxHighlighting() {
    isSyntaxHighlightingEnabled = !isSyntaxHighlightingEnabled;

    // Réappliquer à tous les onglets
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            applyAppropriateHighlighter(textEdit);
        }
    }

    QString status = isSyntaxHighlightingEnabled
        ? "La détection automatique de code C++ est activée"
        : "La détection automatique est désactivée";
    statusBar()->showMessage(status, 3000);
}

// --- Clic Droit ---
void MainWindow::showContextMenu(const QPoint& pos) {
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    if (!textEdit) return;

    QMenu* menu = textEdit->createStandardContextMenu();

    // Afficher les suggestions uniquement si pas en mode code
    if (isCorrectionActive && spellChecker && spellChecker->isValid()
        && textEdit->property("highlighterType").toString() != "cpp") {

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
    isCorrectionActive = s->isSpellCheckEnabled();

    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            textEdit->setFont(font);
            QString style = QString("QPlainTextEdit { color: %1; background-color: white; }")
                .arg(color.name());
            textEdit->setStyleSheet(style);
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