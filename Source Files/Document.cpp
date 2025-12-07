#include "Document.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

Document::Document(QObject* parent)
    : QObject(parent), isModified(false)
{
    initialize();
}

Document::Document(const QString& filePath, QObject* parent)
    : QObject(parent), filePath(filePath), isModified(false)
{
    initialize();
}

Document::~Document()
{
    // Le destructeur est vide car Qt gère automatiquement les objets QObject
}

void Document::initialize()
{
    dateCreated = QDateTime::currentDateTime();
    dateModified = QDateTime::currentDateTime();
    content = "";
}

bool Document::loadFromFile()
{
    if (filePath.isEmpty()) {
        return false;
    }
    return loadFromFile(filePath);
}

bool Document::loadFromFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.exists()) {
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);  // Support UTF-8 (Qt 6)
#else
    in.setCodec("UTF-8");  // Support UTF-8 (Qt 5)
#endif
    content = in.readAll();
    file.close();

    // Mettre à jour le chemin si différent
    if (this->filePath != filePath) {
        this->filePath = filePath;
        emit filePathChanged(filePath);
    }

    // Réinitialiser l'état de modification
    isModified = false;
    dateModified = QDateTime::currentDateTime();

    emit fileLoaded();
    emit contentChanged();
    emit modificationStateChanged(false);

    return true;
}

bool Document::saveToFile()
{
    if (filePath.isEmpty()) {
        return false;
    }
    return saveToFile(filePath);
}

bool Document::saveToFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);  // Support UTF-8 (Qt 6)
#else
    out.setCodec("UTF-8");  // Support UTF-8 (Qt 5)
#endif
    out << content;
    file.close();

    // Mettre à jour le chemin si différent
    if (this->filePath != filePath) {
        this->filePath = filePath;
        emit filePathChanged(filePath);
    }

    // Marquer comme sauvegardé
    isModified = false;
    dateModified = QDateTime::currentDateTime();

    emit fileSaved();
    emit modificationStateChanged(false);

    return true;
}

QString Document::getFilePath() const
{
    return filePath;
}

void Document::setFilePath(const QString& filePath)
{
    if (this->filePath != filePath) {
        this->filePath = filePath;
        emit filePathChanged(filePath);
    }
}

QString Document::getFileName() const
{
    if (filePath.isEmpty()) {
        return "Sans titre";
    }
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}

bool Document::hasFilePath() const
{
    return !filePath.isEmpty();
}

QString Document::getContent() const
{
    return content;
}

void Document::setContent(const QString& content)
{
    if (this->content != content) {
        this->content = content;
        isModified = true;
        dateModified = QDateTime::currentDateTime();
        emit contentChanged();
        emit modificationStateChanged(true);
    }
}

void Document::appendContent(const QString& text)
{
    content += text;
    isModified = true;
    dateModified = QDateTime::currentDateTime();
    emit contentChanged();
    emit modificationStateChanged(true);
}

void Document::clearContent()
{
    if (!content.isEmpty()) {
        content.clear();
        isModified = true;
        dateModified = QDateTime::currentDateTime();
        emit contentChanged();
        emit modificationStateChanged(true);
    }
}

int Document::getContentLength() const
{
    return content.length();
}

int Document::getLineCount() const
{
    if (content.isEmpty()) {
        return 0;
    }
    return content.count('\n') + 1;
}

bool Document::getIsModified() const
{
    return isModified;
}

void Document::setIsModified(bool modified)
{
    if (isModified != modified) {
        isModified = modified;
        if (modified) {
            dateModified = QDateTime::currentDateTime();
        }
        emit modificationStateChanged(isModified);
    }
}

void Document::markAsModified()
{
    setIsModified(true);
}

void Document::markAsSaved()
{
    setIsModified(false);
    dateModified = QDateTime::currentDateTime();
    emit fileSaved();
    emit modificationStateChanged(false);
}

QDateTime Document::getDateCreated() const
{
    return dateCreated;
}

QDateTime Document::getDateModified() const
{
    return dateModified;
}

void Document::updateModificationDate()
{
    dateModified = QDateTime::currentDateTime();
}

bool Document::isEmpty() const
{
    return content.isEmpty();
}

bool Document::exists() const
{
    if (filePath.isEmpty()) {
        return false;
    }
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isFile();
}
