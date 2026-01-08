#include "AIAssistant.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

AIAssistant::AIAssistant(QObject* parent)
    : QObject(parent)
{
}

QString AIAssistant::buildUserContent(const QString& question, const QString& selectedText)
{
    QString content;
    if (!selectedText.trimmed().isEmpty()) {
        content += "Selected text (user selection):\n";
        content += "-----\n";
        content += selectedText;
        content += "\n-----\n\n";
    }
    content += "User question:\n";
    content += question;
    return content;
}

void AIAssistant::askDeepSeek(
    const QString& apiKey,
    const QString& question,
    const QString& selectedText,
    const QString& model
) {
    const QString trimmedKey = apiKey.trimmed();
    if (trimmedKey.isEmpty()) {
        emit requestFailed("Missing DeepSeek API key. Please set it in Settings.");
        return;
    }
    if (question.trimmed().isEmpty()) {
        emit requestFailed("Please enter a question.");
        return;
    }

    // OpenRouter (OpenAI-compatible)
    QUrl url("https://openrouter.ai/api/v1/chat/completions");

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Accept", "application/json");
    req.setRawHeader("Authorization", QString("Bearer %1").arg(trimmedKey).toUtf8());
    // Recommended by OpenRouter for attribution/analytics (can be any valid URL/title)
    req.setRawHeader("HTTP-Referer", "https://uitpad.local");
    req.setRawHeader("X-Title", "UITPad");

    QJsonArray messages;
    messages.append(QJsonObject{
        { "role", "system" },
        { "content", "You are a helpful assistant. Answer clearly and concisely. If code is involved, provide correct code and brief explanations." }
        });
    messages.append(QJsonObject{
        { "role", "user" },
        { "content", buildUserContent(question, selectedText) }
        });

    QJsonObject payload{
        { "model", model },
        { "messages", messages },
        { "temperature", 0.2 },
        { "stream", false }
    };

    QNetworkReply* reply = network.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray raw = reply->readAll();
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QString rawText = QString::fromUtf8(raw).trimmed();

        // Note: Qt may report HTTP 4xx/5xx either as an errorString or as a normal reply.
        if (reply->error() != QNetworkReply::NoError) {
            QString msg = QString("Network error: %1").arg(reply->errorString());
            if (statusCode > 0) msg += QString(" (HTTP %1)").arg(statusCode);
            if (!rawText.isEmpty()) msg += QString("\n%1").arg(rawText);
            emit requestFailed(msg);
            reply->deleteLater();
            return;
        }

        if (statusCode >= 400) {
            QString msg;
            if (statusCode == 401) {
                msg = "OpenRouter error: Unauthorized (HTTP 401). Check your API key (Settings → IA).";
            } else if (statusCode == 402) {
                msg = "OpenRouter error: Payment required / insufficient credits (HTTP 402). Choose a free model or add credits in OpenRouter.";
            } else if (statusCode == 429) {
                msg = "OpenRouter error: Rate limited (HTTP 429). Please wait and try again.";
            } else {
                msg = QString("OpenRouter HTTP error: %1").arg(statusCode);
            }
            if (!rawText.isEmpty()) msg += QString("\n%1").arg(rawText);
            emit requestFailed(msg);
            reply->deleteLater();
            return;
        }

        QJsonParseError err{};
        QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            QString msg = "Failed to parse AI response.";
            if (!rawText.isEmpty()) msg += QString("\n%1").arg(rawText);
            emit requestFailed(msg);
            reply->deleteLater();
            return;
        }

        const QJsonObject obj = doc.object();
        if (obj.contains("error")) {
            const QJsonObject e = obj.value("error").toObject();
            const QString msg = e.value("message").toString("Unknown error");
            emit requestFailed(QString("DeepSeek error: %1").arg(msg));
            reply->deleteLater();
            return;
        }

        const QJsonArray choices = obj.value("choices").toArray();
        if (choices.isEmpty()) {
            emit requestFailed("Empty AI response.");
            reply->deleteLater();
            return;
        }

        const QJsonObject firstChoice = choices.at(0).toObject();
        const QJsonObject message = firstChoice.value("message").toObject();
        const QString answer = message.value("content").toString().trimmed();

        if (answer.isEmpty()) {
            emit requestFailed("AI returned an empty answer.");
            reply->deleteLater();
            return;
        }

        emit answerReady(answer);
        reply->deleteLater();
        });
}

