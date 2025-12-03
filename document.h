#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QString>
#include <QDateTime>

class Document : public QObject
{
    Q_OBJECT

private:
    QString filePath;           // Chemin du fichier (vide si nouveau document)
    QString content;            // Contenu du document
    bool isModified;            // Indique si le document a été modifié
    QDateTime dateCreated;      // Date de création
    QDateTime dateModified;     // Date de dernière modification

public:
    // Constructeurs
    explicit Document(QObject* parent = nullptr);
    explicit Document(const QString& filePath, QObject* parent = nullptr);
    ~Document();

    // Chargement et sauvegarde
    bool loadFromFile();
    bool loadFromFile(const QString& filePath);
    bool saveToFile();
    bool saveToFile(const QString& filePath);

    // Gestion du chemin de fichier
    QString getFilePath() const;
    void setFilePath(const QString& filePath);
    QString getFileName() const;        // Retourne juste le nom du fichier
    bool hasFilePath() const;            // Vérifie si le document a un chemin

    // Gestion du contenu
    QString getContent() const;
    void setContent(const QString& content);
    void appendContent(const QString& text);
    void clearContent();
    int getContentLength() const;
    int getLineCount() const;

    // Gestion de l'état de modification
    bool getIsModified() const;
    void setIsModified(bool modified);
    void markAsModified();
    void markAsSaved();

    // Métadonnées
    QDateTime getDateCreated() const;
    QDateTime getDateModified() const;
    void updateModificationDate();

    // Vérifications
    bool isEmpty() const;
    bool exists() const;                 // Vérifie si le fichier existe sur le disque

signals:
    // Signaux émis lors des changements
    void contentChanged();
    void filePathChanged(const QString& newPath);
    void modificationStateChanged(bool isModified);
    void fileSaved();
    void fileLoaded();

private:
    void initialize();
};

#endif // DOCUMENT_H
