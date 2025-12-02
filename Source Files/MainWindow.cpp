// Source Files/MainWindow.cpp

#include "MainWindow.h"
#include "TextEditor.h"
#include "Document.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequence>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Créer le gestionnaire de documents
    textEditor = new TextEditor(this);
    
    // Créer le widget d'onglets
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);
    
    // Connexion pour le changement d'onglet
    connect(tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onTabCloseRequested);

    // Connexions des signaux TextEditor -> MainWindow
    connect(textEditor, &TextEditor::documentOpened,
            this, &MainWindow::onDocumentOpened);
    connect(textEditor, &TextEditor::documentClosed,
            this, &MainWindow::onDocumentClosed);
    connect(textEditor, &TextEditor::documentSaved,
            this, &MainWindow::onDocumentSaved);
    connect(textEditor, &TextEditor::currentDocumentChanged,
            this, &MainWindow::onCurrentDocumentChanged);
    
    // Note: Les connexions textChanged sont faites individuellement pour chaque onglet
    // dans onDocumentOpened() quand un document est ouvert

    // Menu Fichier
    QMenu* fileMenu = menuBar()->addMenu("Fichier");
    fileMenu->addAction("Nouveau", QKeySequence::New, this, &MainWindow::onFileNew);
    fileMenu->addAction("Ouvrir", QKeySequence::Open, this, &MainWindow::onFileOpen);
    fileMenu->addSeparator();
    fileMenu->addAction("Sauvegarder", QKeySequence::Save, this, &MainWindow::onFileSave);
    fileMenu->addAction("Sauvegarder sous...", QKeySequence::SaveAs, this, &MainWindow::onFileSaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Fermer", QKeySequence::Close, this, &MainWindow::onFileClose);
    fileMenu->addAction("Quitter", QKeySequence::Quit, this, &QWidget::close);

    // Titre initial
    setWindowTitle("UITPad - Sans titre");
}

MainWindow::~MainWindow() {
    // textEditor sera supprimé automatiquement car parent = this
}

void MainWindow::onFileNew() {
    Document* doc = textEditor->createNewDocument();
    // Le signal documentOpened sera émis automatiquement
}

void MainWindow::onFileOpen() {
    QString filename = QFileDialog::getOpenFileName(this, "Ouvrir un fichier");
    if (filename.isEmpty()) return;

    Document* doc = textEditor->openDocument(filename);
    if (!doc) {
        QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier.");
    }
    // Le signal documentOpened sera émis automatiquement
}

void MainWindow::onFileSave() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) {
        QMessageBox::information(this, "Information", "Aucun document ouvert.");
        return;
    }

    // Si le document n'a pas de chemin, demander où sauvegarder
    if (!doc->hasFilePath()) {
        onFileSaveAs();
        return;
    }

    // Sauvegarder le document
    if (textEditor->saveDocument(doc)) {
        // Le signal documentSaved sera émis automatiquement
    } else {
        QMessageBox::warning(this, "Erreur", "Impossible de sauvegarder le fichier.");
    }
}

void MainWindow::onFileSaveAs() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) {
        QMessageBox::information(this, "Information", "Aucun document ouvert.");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, "Sauvegarder sous...");
    if (filename.isEmpty()) return;

    // Définir le nouveau chemin
    doc->setFilePath(filename);
    
    // Sauvegarder
    if (textEditor->saveDocument(doc)) {
        // Le signal documentSaved sera émis automatiquement
    } else {
        QMessageBox::warning(this, "Erreur", "Impossible de sauvegarder le fichier.");
    }
}

void MainWindow::onFileClose() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) {
        return;
    }

    // Vérifier si le document a été modifié
    if (doc->getIsModified()) {
        QString fileName = doc->getFileName();
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Document modifié",
            QString("Le document \"%1\" a été modifié. Voulez-vous sauvegarder les modifications?")
                .arg(fileName),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            onFileSave();
            // Continuer à fermer même si la sauvegarde échoue
        } else if (reply == QMessageBox::Cancel) {
            return; // Annuler la fermeture
        }
    }

    // Fermer le document
    textEditor->closeDocument(doc);
    // Le signal documentClosed sera émis automatiquement et l'onglet sera fermé
}

void MainWindow::onDocumentOpened(Document* doc) {
    if (doc) {
        // Créer un QPlainTextEdit pour ce document
        QPlainTextEdit* textEdit = new QPlainTextEdit(this);
        textEdit->setPlainText(doc->getContent());
        
        // Ajouter l'onglet
        int index = tabWidget->addTab(textEdit, doc->getFileName());
        tabWidget->setCurrentIndex(index);
        
        // Stocker le Document* dans les données de l'onglet
        textEdit->setProperty("document", QVariant::fromValue(doc));
        
        // Connexion pour détecter les modifications
        connect(textEdit, &QPlainTextEdit::textChanged,
                this, &MainWindow::onTextChanged);
        
        // Mettre à jour le titre de l'onglet
        updateTabTitle(doc);
        updateWindowTitle();
    }
}

void MainWindow::onDocumentClosed(Document* doc) {
    // Trouver et fermer l'onglet correspondant
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
    
    // Mettre à jour l'affichage
    updateWindowTitle();
}

void MainWindow::onDocumentSaved(Document* doc) {
    // Mettre à jour le titre de l'onglet et de la fenêtre (retirer l'astérisque)
    if (doc) {
        updateTabTitle(doc);
    }
    updateWindowTitle();
    QMessageBox::information(this, "Sauvegarde", "Fichier sauvegardé avec succès.");
}

void MainWindow::onCurrentDocumentChanged(Document* doc) {
    // Trouver et activer l'onglet correspondant
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
    // Mettre à jour le contenu du document actuel
    QPlainTextEdit* textEdit = getCurrentTextEdit();
    if (textEdit) {
        Document* doc = textEdit->property("document").value<Document*>();
        if (doc) {
            // Mettre à jour le contenu du document
            doc->setContent(textEdit->toPlainText());
            
            // Mettre à jour le titre de l'onglet et de la fenêtre
            updateTabTitle(doc);
            updateWindowTitle();
        }
    }
}

void MainWindow::updateTextAreaContent() {
    Document* doc = textEditor->getCurrentDocument();
    QPlainTextEdit* textEdit = getTextEditForDocument(doc);
    if (textEdit && doc) {
        // Désactiver temporairement le signal pour éviter de déclencher onTextChanged
        textEdit->blockSignals(true);
        textEdit->setPlainText(doc->getContent());
        textEdit->blockSignals(false);
    }
}

void MainWindow::onTabChanged(int index) {
    if (index >= 0 && index < tabWidget->count()) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
        if (textEdit) {
            Document* doc = textEdit->property("document").value<Document*>();
            if (doc) {
                // Basculer vers ce document dans TextEditor
                textEditor->switchToDocument(doc);
            }
        }
    }
}

void MainWindow::onTabCloseRequested(int index) {
    if (index >= 0 && index < tabWidget->count()) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
        if (textEdit) {
            Document* doc = textEdit->property("document").value<Document*>();
            if (doc) {
                // Fermer le document (demandera confirmation si modifié)
                onFileClose();
            }
        }
    }
}

QPlainTextEdit* MainWindow::getCurrentTextEdit() const {
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0 && currentIndex < tabWidget->count()) {
        return qobject_cast<QPlainTextEdit*>(tabWidget->widget(currentIndex));
    }
    return nullptr;
}

QPlainTextEdit* MainWindow::getTextEditForDocument(Document* doc) const {
    if (!doc) return nullptr;
    
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            Document* tabDoc = textEdit->property("document").value<Document*>();
            if (tabDoc == doc) {
                return textEdit;
            }
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
            if (doc->getIsModified()) {
                title = "*" + title;
            }
            tabWidget->setTabText(index, title);
        }
    }
}

void MainWindow::updateWindowTitle() {
    Document* doc = textEditor->getCurrentDocument();
    if (doc) {
        QString title = doc->getFileName();
        if (doc->getIsModified()) {
            title = "*" + title;
        }
        title += " - UITPad";
        setWindowTitle(title);
    } else {
        setWindowTitle("UITPad - Sans titre");
    }
}