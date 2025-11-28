#include "TextEditor.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

TextEditor::TextEditor(QObject* parent)
    : QObject(parent), currentDocumentIndex(-1) {
}

TextEditor::~TextEditor() {
    // Libérer la mémoire
    for (Document* doc : documents) {
        delete doc;
    }
}

// ========================================
// CRÉER UN NOUVEAU DOCUMENT
// ========================================
void TextEditor::newDocument() {
    Document* doc = new Document(this);
    doc->setFilename("Sans titre " + QString::number(documents.size() + 1));

    documents.append(doc);
    currentDocumentIndex = documents.size() - 1;

    emit documentOpened(currentDocumentIndex, doc->getFilename());
    emit currentDocumentChanged(currentDocumentIndex);
}

// ========================================
// OUVRIR UN DOCUMENT
// ========================================
void TextEditor::openDocument(const QString& filepath) {
    // Vérifier si le fichier est déjà ouvert
    for (int i = 0; i < documents.size(); i++) {
        if (documents[i]->getFilepath() == filepath) {
            // Fichier déjà ouvert, basculer dessus
            setCurrentDocument(i);
            return;
        }
    }

    // Créer un nouveau document
    Document* doc = new Document(this);

    // Charger le fichier
    if (doc->open(filepath)) {
        documents.append(doc);
        currentDocumentIndex = documents.size() - 1;

        QFileInfo fileInfo(filepath);
        emit documentOpened(currentDocumentIndex, fileInfo.fileName());
        emit currentDocumentChanged(currentDocumentIndex);
    }
    else {
        delete doc;  // Échec du chargement
    }
}

// ========================================
// FERMER UN DOCUMENT
// ========================================
void TextEditor::closeDocument(int index) {
    if (index < 0 || index >= documents.size()) {
        return;
    }

    // Supprimer le document
    Document* doc = documents[index];
    documents.remove(index);
    delete doc;

    emit documentClosed(index);

    // Ajuster l'index du document actif
    if (documents.isEmpty()) {
        currentDocumentIndex = -1;
    }
    else if (currentDocumentIndex >= documents.size()) {
        currentDocumentIndex = documents.size() - 1;
        emit currentDocumentChanged(currentDocumentIndex);
    }
}

// ========================================
// SAUVEGARDER UN DOCUMENT
// ========================================
void TextEditor::saveDocument(int index) {
    if (index >= 0 && index < documents.size()) {
        documents[index]->save();
        emit documentModified(index, false);
    }
}

// ========================================
// SAUVEGARDER TOUS LES DOCUMENTS
// ========================================
void TextEditor::saveAllDocuments() {
    for (int i = 0; i < documents.size(); i++) {
        if (documents[i]->isModified()) {
            saveDocument(i);
        }
    }
}

// ========================================
// CHANGER DE DOCUMENT ACTIF
// ========================================
void TextEditor::setCurrentDocument(int index) {
    if (index >= 0 && index < documents.size()) {
        currentDocumentIndex = index;
        emit currentDocumentChanged(index);
    }
}

// ========================================
// RÉCUPÉRER LE DOCUMENT ACTIF
// ========================================
Document* TextEditor::getCurrentDocument() {
    if (currentDocumentIndex >= 0 && currentDocumentIndex < documents.size()) {
        return documents[currentDocumentIndex];
    }
    return nullptr;
}

// ========================================
// RÉCUPÉRER UN DOCUMENT PAR INDEX
// ========================================
Document* TextEditor::getDocument(int index) {
    if (index >= 0 && index < documents.size()) {
        return documents[index];
    }
    return nullptr;
}

// ========================================
// NOMBRE DE DOCUMENTS OUVERTS
// ========================================
int TextEditor::getDocumentCount() {
    return documents.size();
}

// ========================================
// INDEX DU DOCUMENT ACTIF
// ========================================
int TextEditor::getCurrentDocumentIndex() {
    return currentDocumentIndex;
}

// ========================================
// TITRE D'UN DOCUMENT
// ========================================
QString TextEditor::getDocumentTitle(int index) {
    if (index >= 0 && index < documents.size()) {
        return documents[index]->getFilename();
    }
    return "";
}

// ========================================
// DOCUMENT MODIFIÉ ?
// ========================================
bool TextEditor::isDocumentModified(int index) {
    if (index >= 0 && index < documents.size()) {
        return documents[index]->isModified();
    }
    return false;
}

// ========================================
// OPÉRATIONS TEXTE (sur document actif)
// ========================================
void TextEditor::cut() {
    // Sera géré par QPlainTextEdit dans MainWindow
}

void TextEditor::copy() {
    // Sera géré par QPlainTextEdit dans MainWindow
}

void TextEditor::paste() {
    // Sera géré par QPlainTextEdit dans MainWindow
}

