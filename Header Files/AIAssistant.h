#ifndef AIASSISTANT_H
#define AIASSISTANT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

class AIAssistant : public QObject
{
    Q_OBJECT

public:
    explicit AIAssistant(QObject *parent = nullptr);
    ~AIAssistant();


    void setApiKey(const QString &apiKey);
    QString getApiKey() const { return apiKey; }
    bool isConfigured() const { return !apiKey.isEmpty(); }

    // Fonctions principales
    void askQuestion(const QString &question);
    void explainCode(const QString &code);
    void generateCode(const QString &prompt);
    void completeCode(const QString &context);

signals:
    void responseReady(const QString &response);
    void errorOccurred(const QString &error);
    void statusMessage(const QString &message);

private slots:
    void onApiResponse(QNetworkReply *reply);

private:
    void saveSettings();
    void loadSettings();

    QNetworkAccessManager *networkManager;
    QSettings *settings;
    QString apiKey;
};

#endif // AIASSISTANT_H
