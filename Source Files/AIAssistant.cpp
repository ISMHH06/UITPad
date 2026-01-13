#include "AIAssistant.h"
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

AIAssistant::AIAssistant(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
    , settings(new QSettings("CodeMind", "AIAssistant", this))
{
    loadSettings();
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &AIAssistant::onApiResponse);
}

AIAssistant::~AIAssistant() {
    saveSettings();
}

void AIAssistant::loadSettings() {
    apiKey = settings->value("apiKey", "").toString();
}

void AIAssistant::saveSettings() {
    settings->setValue("apiKey", apiKey);
    settings->sync();
}

void AIAssistant::setApiKey(const QString &key) {
    apiKey = key.trimmed();
    saveSettings();
    emit statusMessage("Clé API configurée");
}

void AIAssistant::askQuestion(const QString &question) {
    if (!isConfigured()) {
        emit errorOccurred("Configurez d'abord votre clé API dans Paramètres → IA");
        return;
    }

    emit statusMessage("Envoi à l'IA...");

    QJsonObject request;
    request["model"] = "gpt-4o-mini";
    request["temperature"] = 0.2;
    request["max_tokens"] = 1000;

    QJsonArray messages;
    QJsonObject systemMsg, userMsg;

    systemMsg["role"] = "system";
    systemMsg["content"] = "Tu es CodeMind, assistant de programmation pour le langage c++. Réponds en français, sois concis.";

    userMsg["role"] = "user";
    userMsg["content"] = question;

    messages.append(systemMsg);
    messages.append(userMsg);
    request["messages"] = messages;

    QNetworkRequest netRequest;
    netRequest.setUrl(QUrl("https://openrouter.ai/api/v1/chat/completions"));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    netRequest.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    QJsonDocument doc(request);
    networkManager->post(netRequest, doc.toJson());
}

void AIAssistant::explainCode(const QString &code) {
    QString prompt = QString("Explique ce code de manière concise:\n```\n%1\n```").arg(code);
    askQuestion(prompt);
}

void AIAssistant::generateCode(const QString &prompt) {
    QString fullPrompt = QString("Génère du code pour: %1\nRéponds SEULEMENT avec le code, sans explications.").arg(prompt);
    askQuestion(fullPrompt);
}

void AIAssistant::completeCode(const QString &context) {
    QString prompt = QString("Complète ce code:\n```\n%1\n```\n\nRéponds SEULEMENT avec la suite du code.").arg(context);
    askQuestion(prompt);
}

void AIAssistant::onApiResponse(QNetworkReply *reply) {
    if (reply->error()) {
        emit errorOccurred("Erreur: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emit errorOccurred("Réponse invalide");
        return;
    }

    QJsonObject root = doc.object();

    if (root.contains("error")) {
        QJsonObject error = root["error"].toObject();
        emit errorOccurred("API: " + error["message"].toString());
        return;
    }

    QString response;
    if (root.contains("choices") && root["choices"].isArray()) {
        QJsonArray choices = root["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            if (choice.contains("message")) {
                QJsonObject message = choice["message"].toObject();
                response = message["content"].toString().trimmed();
            }
        }
    }

    if (response.isEmpty()) {
        emit errorOccurred("Réponse vide");
        return;
    }

    emit responseReady(response);
    emit statusMessage("Réponse reçue");
}
