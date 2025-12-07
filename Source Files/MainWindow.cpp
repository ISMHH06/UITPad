#include "MainWindow.h"
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"
#include "highlighter.h"

#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequence>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QTextCursor>

// --- NOUVEAUX INCLUDES POUR RENOMMER ---
#include <QInputDialog>
#include <QFileInfo>
#include <QDir>
// ---------------------------------------

// --- Constructeur ---
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // 1. Initialisation du correcteur via les ressources
    spellChecker = new SpellChecker(":/dictionary.txt");

    // 2. Gestionnaire de documents
    textEditor = new TextEditor(this);

    // 3. Onglets
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);

    // Connexions Onglets
    connect(tabWidget, &QTabWidget::currentChanged,
        this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested,
        this, &MainWindow::onTabCloseRequested);

    // Connexions TextEditor
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

    // --- AJOUT DE L'OPTION RENOMMER ---
    fileMenu->addAction("Renommer...", this, &MainWindow::onFileRename);
    // ----------------------------------

    fileMenu->addSeparator();
    fileMenu->addAction("Sauvegarder", QKeySequence::Save, this, &MainWindow::onFileSave);
    fileMenu->addAction("Sauvegarder sous...", QKeySequence::SaveAs, this, &MainWindow::onFileSaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Fermer", QKeySequence::Close, this, &MainWindow::onFileClose);
    fileMenu->addAction("Quitter", QKeySequence::Quit, this, &QWidget::close);

    // Menu Édition pour les Paramètres
    QMenu* editMenu = menuBar()->addMenu("Édition");
    editMenu->addAction("Paramètres...", this, &MainWindow::onOpenSettings);

    setWindowTitle("UITPad - Sans titre");
}

// --- Destructeur ---
MainWindow::~MainWindow() {
    delete spellChecker;
}

// --- Gestion Fichiers ---
void MainWindow::onFileNew() {
    textEditor->createNewDocument();
}

void MainWindow::onFileOpen() {
    QString filename = QFileDialog::getOpenFileName(this, "Ouvrir un fichier");
    if (filename.isEmpty()) return;

    Document* doc = textEditor->openDocument(filename);
    if (!doc) QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier.");
}

// --- NOUVELLE FONCTION : RENOMMER ---
void MainWindow::onFileRename() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

    // Si le fichier n'est pas encore sauvegardé, "Renommer" = "Sauvegarder sous"
    if (!doc->hasFilePath()) {
        onFileSaveAs();
        return;
    }

    QString oldPath = doc->getFilePath();
    QFileInfo fileInfo(oldPath);
    QString oldName = fileInfo.fileName();

    // Demander le nouveau nom
    bool ok;
    QString newName = QInputDialog::getText(this, "Renommer le fichier",
        "Nouveau nom :", QLineEdit::Normal,
        oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        QString newPath = fileInfo.absolutePath() + "/" + newName;
        QFile file(oldPath);

        // On essaie de renommer sur le disque
        if (file.rename(newPath)) {
            // Mise à jour du document et de l'interface
            doc->setFilePath(newPath);
            updateTabTitle(doc);
            updateWindowTitle();
            QMessageBox::information(this, "Succès", "Fichier renommé avec succès.");
        }
        else {
            QMessageBox::warning(this, "Erreur", "Impossible de renommer le fichier.\n(Vérifiez qu'il n'est pas ouvert ailleurs).");
        }
    }
}
// ------------------------------------

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

        // Soulignement rouge automatique
        if (spellChecker) {
            new Highlighter(textEdit->document(), spellChecker);
        }

        // Menu contextuel (Clic droit)
        textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(textEdit, &QPlainTextEdit::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

        // Appliquer la police des paramètres
        Settings s(this);
        textEdit->setFont(s.getEditorFont());

        int index = tabWidget->addTab(textEdit, doc->getFileName());
        tabWidget->setCurrentIndex(index);
        textEdit->setProperty("document", QVariant::fromValue(doc));

        connect(textEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::onTextChanged);

        updateTabTitle(doc);
        updateWindowTitle();
    }
}

// --- Clic Droit Intelligent ---
void MainWindow::showContextMenu(const QPoint& pos) {
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    if (!textEdit) return;

    QMenu* menu = textEdit->createStandardContextMenu();

    if (isCorrectionActive && spellChecker && spellChecker->isValid()) {
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

// --- Paramètres ---
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
            QString style = QString("QPlainTextEdit { color: %1; background-color: white; selection-background-color: #0078d7; selection-color: white; }")
                .arg(color.name());
            textEdit->setStyleSheet(style);
        }
    }
}

// --- Autres fonctions utilitaires ---
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
