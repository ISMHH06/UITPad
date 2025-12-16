#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QObject>
#include <QVector>
#include <QString>
class Document;

class TextEditor : public QObject
{
    Q_OBJECT

private:
    QVector<Document*> documents;      // Liste de tous les documents ouverts
    int currentDocumentIndex;           // Index du document actuellement actif (-1 si aucun)

public:
    explicit TextEditor(QObject* parent = nullptr);
    ~TextEditor();

    // Gestion des documents
    Document* openDocument(const QString& filePath);
    Document* createNewDocument();
    bool closeDocument(Document* doc);
    bool closeDocument(int index);
    bool saveDocument(Document* doc);
    bool saveDocument(int index);
    bool saveAllDocuments();

    // Navigation entre documents
    bool switchToDocument(int index);
    bool switchToDocument(Document* doc);
    bool switchToNextDocument();
    bool switchToPreviousDocument();

    // Accès aux documents
    Document* getCurrentDocument() const;
    Document* getDocument(int index) const;
    int getCurrentDocumentIndex() const;
    int getDocumentCount() const;
    int findDocumentIndex(const QString& filePath) const;
    bool isDocumentOpen(const QString& filePath) const;

    // Vérifications
    bool hasDocuments() const;
    bool hasUnsavedDocuments() const;

signals:
    // Signaux émis lors des changements de documents
    void documentOpened(Document* doc);
    void documentClosed(Document* doc);
    void documentSaved(Document* doc);
    void currentDocumentChanged(Document* doc);
    void documentModified(Document* doc);

private:
    void updateCurrentDocumentIndex();
    void removeDocument(Document* doc);
};

#endif // TEXTEDITOR_H
