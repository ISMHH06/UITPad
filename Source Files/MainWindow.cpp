#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), zoomLevel(100) {

    // Créer le TextEditor (gère plusieurs documents)
    textEditor = new TextEditor(this);

    spellChecker = new SpellChecker(this);
    aiAssistant = new AIAssistant(this);
    settings = new Settings(this);

    createUI();
    createMenus();
    connectSignals();

    // Créer un document initial
    onFileNew();
}

// ========================================
// CRÉER L'INTERFACE
// ========================================
void MainWindow::createUI() {
    // Onglets pour les fichiers
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);  // Bouton de fermeture sur chaque onglet
    tabWidget->setMovable(true);       // Onglets déplaçables
    setCentralWidget(tabWidget);

    // Barre d'état
    statusLabel = new QLabel("UTF-8 | Zoom: 100%", this);
    statusBar()->addPermanentWidget(statusLabel);
}

// ========================================
// CRÉER LES MENUS
// ========================================
void MainWindow::createMenus() {
    // Menu Fichier
    QMenu* fileMenu = menuBar()->addMenu("Fichier");

    fileMenu->addAction("Nouveau", this, &MainWindow::onFileNew, QKeySequence::New);
    fileMenu->addAction("Ouvrir", this, &MainWindow::onFileOpen, QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("Sauvegarder", this, &MainWindow::onFileSave, QKeySequence::Save);
    fileMenu->addAction("Sauvegarder sous...", this, &MainWindow::onFileSaveAs);
    fileMenu->addAction("Sauvegarder tout", this, &MainWindow::onFileSaveAll);
    fileMenu->addSeparator();
    fileMenu->addAction("Fermer", this, &MainWindow::onFileClose, QKeySequence::Close);
    fileMenu->addAction("Fermer tout", this, &MainWindow::onFileCloseAll);
    fileMenu->addSeparator();
    fileMenu->addAction("Quitter", this, &QWidget::close, QKeySequence::Quit);

    // Menu Affichage
    QMenu* viewMenu = menuBar()->addMenu("Affichage");
    viewMenu->addAction("Thème Dark", this, &MainWindow::onThemeDark);
    viewMenu->addAction("Thème Light", this, &MainWindow::onThemeLight);
}

// ========================================
// CONNECTER LES SIGNAUX
// ========================================
void MainWindow::connectSignals() {
    // Signaux des onglets
    connect(tabWidget, &QTabWidget::currentChanged,
        this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested,
        this, &MainWindow::onTabCloseRequested);

    // Signaux du TextEditor
    connect(textEditor, &TextEditor::documentOpened,
        this, &MainWindow::onDocumentOpened);
    connect(textEditor, &TextEditor::documentClosed,
        this, &MainWindow::onDocumentClosed);
    connect(textEditor, &TextEditor::currentDocumentChanged,
        this, &MainWindow::onCurrentDocumentChanged);
}

// ========================================
// NOUVEAU DOCUMENT
// ========================================
void MainWindow::onFileNew() {
    textEditor->newDocument();
}

void MainWindow::onDocumentOpened(int index, QString title) {
    // Créer une nouvelle zone de texte
    QPlainTextEdit* textArea = new QPlainTextEdit();

    // Créer le highlighter
    CppSyntaxHighlighter* highlighter = new CppSyntaxHighlighter(textArea->document());

    // Stocker
    textAreas[index] = textArea;
    highlighters[index] = highlighter;

    // Ajouter l'onglet
    tabWidget->addTab(textArea, title);
    tabWidget->setCurrentIndex(tabWidget->count() - 1);

    // Connecter les signaux
    connect(textArea, &QPlainTextEdit::textChanged,
        this, &MainWindow::onTextChanged);

    statusBar()->showMessage("Document créé : " + title, 2000);
}

// ========================================
// OUVRIR UN DOCUMENT
// ========================================
void MainWindow::onFileOpen() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Ouvrir un fichier",
        "",
        "Fichiers C++ (*.cpp *.h *.hpp);;Fichiers texte (*.txt);;Tous (*.*)"
    );

    if (!filename.isEmpty()) {
        textEditor->openDocument(filename);
    }
}

// ========================================
// SAUVEGARDER LE DOCUMENT ACTIF
// ========================================
void MainWindow::onFileSave() {
    int index = textEditor->getCurrentDocumentIndex();
    if (index >= 0) {
        Document* doc = textEditor->getCurrentDocument();
        doc->setText(textAreas[index]->toPlainText());
        textEditor->saveDocument(index);

        statusBar()->showMessage("Fichier sauvegardé", 2000);
    }
}

// ========================================
// SAUVEGARDER TOUS LES DOCUMENTS
// ========================================
void MainWindow::onFileSaveAll() {
    for (int i = 0; i < textEditor->getDocumentCount(); i++) {
        Document* doc = textEditor->getDocument(i);
        doc->setText(textAreas[i]->toPlainText());
        textEditor->saveDocument(i);
    }

    statusBar()->showMessage("Tous les fichiers sauvegardés", 2000);
}

// ========================================
// FERMER L'ONGLET ACTIF
// ========================================
void MainWindow::onFileClose() {
    int index = tabWidget->currentIndex();
    onTabCloseRequested(index);
}

void MainWindow::onTabCloseRequested(int tabIndex) {
    // Vérifier si le document est modifié
    int docIndex = textEditor->getCurrentDocumentIndex();

    if (textEditor->isDocumentModified(docIndex)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Sauvegarder ?",
            "Le document a été modifié. Voulez-vous le sauvegarder ?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            onFileSave();
        }
        else if (reply == QMessageBox::Cancel) {
            return;  // Annuler la fermeture
        }
    }

    // Supprimer l'onglet
    QWidget* widget = tabWidget->widget(tabIndex);
    tabWidget->removeTab(tabIndex);
    delete widget;

    // Supprimer des maps
    textAreas.remove(docIndex);
    highlighters.remove(docIndex);

    // Fermer le document
    textEditor->closeDocument(docIndex);
}

// ========================================
// CHANGEMENT D'ONGLET
// ========================================
void MainWindow::onTabChanged(int tabIndex) {
    if (tabIndex >= 0) {
        textEditor->setCurrentDocument(tabIndex);
    }
}

void MainWindow::onCurrentDocumentChanged(int index) {
    // Mettre à jour l'UI selon le nouveau document actif
    Document* doc = textEditor->getCurrentDocument();
    if (doc && textAreas.contains(index)) {
        textAreas[index]->setPlainText(doc->getText());
    }
}

// ========================================
// TEXTE MODIFIÉ
// ========================================
void MainWindow::onTextChanged() {
    int index = textEditor->getCurrentDocumentIndex();
    if (index >= 0) {
        Document* doc = textEditor->getCurrentDocument();
        doc->setText(textAreas[index]->toPlainText());

        // Marquer l'onglet comme modifié
        QString title = textEditor->getDocumentTitle(index);
        if (doc->isModified() && !title.endsWith("*")) {
            tabWidget->setTabText(tabWidget->currentIndex(), title + " *");
        }
    }
}

// Destructeur
MainWindow::~MainWindow() {
    delete textEditor;
    delete spellChecker;
    delete aiAssistant;
    delete settings;
}


