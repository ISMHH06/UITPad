#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QObject>
#include <QVector>
#include <QString>
#include "Document.h"

class TextEditor : public QObject {
    Q_OBJECT

private:
    QVector<Document*> documents;     // Liste de TOUS les documents ouverts
    int currentDocumentIndex;         // Index du document actif

public:
    explicit TextEditor(QObject* parent = nullptr);
    ~TextEditor();

    // Gestion des documents
    void newDocument();                           // Créer un nouveau document
    void openDocument(const QString& filepath);   // Ouvrir un fichier
    void closeDocument(int index);                // Fermer un document
    void saveDocument(int index);                 // Sauvegarder un document
    void saveAllDocuments();                      // Sauvegarder tous les documents

    // Navigation entre documents
    void setCurrentDocument(int index);           // Changer de document actif
    Document* getCurrentDocument();               // Récupérer le document actif
    Document* getDocument(int index);             // Récupérer un document par index

    // Informations
    int getDocumentCount();                       // Nombre de documents ouverts
    int getCurrentDocumentIndex();                // Index du document actif
    QString getDocumentTitle(int index);          // Titre d'un document (nom fichier)
    bool isDocumentModified(int index);           // Document modifié ?

    // Opérations texte
    void cut();                                   // Couper (sur document actif)
    void copy();                                  // Copier
    void paste();                                 // Coller

signals:
    void documentOpened(int index, QString title);     // Signal : document ouvert
    void documentClosed(int index);                    // Signal : document fermé
    void documentModified(int index, bool modified);   // Signal : document modifié
    void currentDocumentChanged(int index);            // Signal : changement de document
};

#endif