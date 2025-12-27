#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>

class QNetworkReply;

class AIAssistant : public QObject
{
    Q_OBJECT

public:
    explicit AIAssistant(QObject* parent = nullptr);

    void askDeepSeek(
        const QString& apiKey,
        const QString& question,
        const QString& selectedText,
        const QString& model = "deepseek-chat"
    );

signals:
    void answerReady(const QString& answer);
    void requestFailed(const QString& errorMessage);

private:
    QNetworkAccessManager network;

    static QString buildUserContent(const QString& question, const QString& selectedText);
};
