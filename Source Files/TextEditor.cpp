#include "TextEditor.h"
#include "Document.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

TextEditor::TextEditor(QObject* parent)
    : QObject(parent), currentDocumentIndex(-1)
{
}

TextEditor::~TextEditor()
{
    // Supprimer tous les documents
    for (Document* doc : documents) {
        delete doc;
    }
    documents.clear();
}

Document* TextEditor::openDocument(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return nullptr;
    }

    // Vérifier si le fichier est déjà ouvert
    int existingIndex = findDocumentIndex(filePath);
    if (existingIndex != -1) {
        // Le document est déjà ouvert, basculer vers lui
        switchToDocument(existingIndex);
        return documents[existingIndex];
    }

    // Créer un nouveau document avec le chemin du fichier
    Document* doc = new Document(filePath);
    
    // Tenter de charger le fichier
    if (!doc->loadFromFile()) {
        delete doc;
        return nullptr;
    }

    // Ajouter le document à la liste
    documents.append(doc);
    currentDocumentIndex = documents.size() - 1;

    // Émettre le signal
    emit documentOpened(doc);
    emit currentDocumentChanged(doc);

    return doc;
}

Document* TextEditor::createNewDocument()
{
    // Créer un nouveau document vide
    Document* doc = new Document();
    
    // Ajouter à la liste
    documents.append(doc);
    currentDocumentIndex = documents.size() - 1;

    // Émettre le signal
    emit documentOpened(doc);
    emit currentDocumentChanged(doc);

    return doc;
}

bool TextEditor::closeDocument(Document* doc)
{
    if (!doc) {
        return false;
    }

    int index = documents.indexOf(doc);
    if (index == -1) {
        return false;
    }

    return closeDocument(index);
}

bool TextEditor::closeDocument(int index)
{
    if (index < 0 || index >= documents.size()) {
        return false;
    }

    Document* doc = documents[index];
    
    // Retirer de la liste
    documents.removeAt(index);
    
    // Mettre à jour l'index courant
    updateCurrentDocumentIndex();

    // Émettre le signal avant de supprimer
    emit documentClosed(doc);
    
    // Si c'était le document courant, émettre le changement
    if (currentDocumentIndex >= 0 && currentDocumentIndex < documents.size()) {
        emit currentDocumentChanged(documents[currentDocumentIndex]);
    } else if (documents.isEmpty()) {
        emit currentDocumentChanged(nullptr);
    }

    // Supprimer le document
    delete doc;

    return true;
}

bool TextEditor::saveDocument(Document* doc)
{
    if (!doc) {
        return false;
    }

    int index = documents.indexOf(doc);
    if (index == -1) {
        return false;
    }

    return saveDocument(index);
}

bool TextEditor::saveDocument(int index)
{
    if (index < 0 || index >= documents.size()) {
        return false;
    }

    Document* doc = documents[index];
    
    // Vérifier si le document a un chemin de fichier
    if (!doc->hasFilePath()) {
        // Nouveau fichier, besoin d'un chemin
        // Cette logique devrait être gérée par MainWindow avec QFileDialog
        return false;
    }

    // Sauvegarder le fichier
    if (doc->saveToFile()) {
        emit documentSaved(doc);
        return true;
    }

    return false;
}

bool TextEditor::saveAllDocuments()
{
    bool allSaved = true;
    for (int i = 0; i < documents.size(); ++i) {
        // Sauvegarder uniquement les documents modifiés qui ont un chemin
        if (documents[i]->getIsModified() && documents[i]->hasFilePath()) {
            if (!saveDocument(i)) {
                allSaved = false;
            }
        }
    }
    return allSaved;
}

bool TextEditor::switchToDocument(int index)
{
    if (index < 0 || index >= documents.size()) {
        return false;
    }

    if (currentDocumentIndex == index) {
        return true; // Déjà sur ce document
    }

    currentDocumentIndex = index;
    emit currentDocumentChanged(documents[index]);

    return true;
}

bool TextEditor::switchToDocument(Document* doc)
{
    if (!doc) {
        return false;
    }

    int index = documents.indexOf(doc);
    if (index == -1) {
        return false;
    }

    return switchToDocument(index);
}

bool TextEditor::switchToNextDocument()
{
    if (documents.isEmpty()) {
        return false;
    }

    int nextIndex = (currentDocumentIndex + 1) % documents.size();
    return switchToDocument(nextIndex);
}

bool TextEditor::switchToPreviousDocument()
{
    if (documents.isEmpty()) {
        return false;
    }

    int prevIndex = (currentDocumentIndex - 1 + documents.size()) % documents.size();
    return switchToDocument(prevIndex);
}

Document* TextEditor::getCurrentDocument() const
{
    if (currentDocumentIndex >= 0 && currentDocumentIndex < documents.size()) {
        return documents[currentDocumentIndex];
    }
    return nullptr;
}

Document* TextEditor::getDocument(int index) const
{
    if (index >= 0 && index < documents.size()) {
        return documents[index];
    }
    return nullptr;
}

int TextEditor::getCurrentDocumentIndex() const
{
    return currentDocumentIndex;
}

int TextEditor::getDocumentCount() const
{
    return documents.size();
}

int TextEditor::findDocumentIndex(const QString& filePath) const
{
    if (filePath.isEmpty()) {
        return -1;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return -1;
    }
    
    QString canonicalPath = fileInfo.canonicalFilePath();

    for (int i = 0; i < documents.size(); ++i) {
        QString docPath = documents[i]->getFilePath();
        if (!docPath.isEmpty()) {
            QFileInfo docFileInfo(docPath);
            if (docFileInfo.exists() && docFileInfo.canonicalFilePath() == canonicalPath) {
                return i;
            }
        }
    }

    return -1;
}

bool TextEditor::isDocumentOpen(const QString& filePath) const
{
    return findDocumentIndex(filePath) != -1;
}

bool TextEditor::hasDocuments() const
{
    return !documents.isEmpty();
}

bool TextEditor::hasUnsavedDocuments() const
{
    for (Document* doc : documents) {
        if (doc->getIsModified()) {
            return true;
        }
    }
    return false;
}

void TextEditor::updateCurrentDocumentIndex()
{
    // Ajuster l'index courant après suppression
    if (currentDocumentIndex >= documents.size()) {
        currentDocumentIndex = documents.size() - 1;
    }
    
    if (currentDocumentIndex < 0 && !documents.isEmpty()) {
        currentDocumentIndex = 0;
    }
    
    if (documents.isEmpty()) {
        currentDocumentIndex = -1;
    }
}

void TextEditor::removeDocument(Document* doc)
{
    int index = documents.indexOf(doc);
    if (index != -1) {
        documents.removeAt(index);
        updateCurrentDocumentIndex();
    }
}
