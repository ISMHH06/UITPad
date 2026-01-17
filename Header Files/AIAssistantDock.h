#pragma once

#include <QDockWidget>
#include <QString>

class AIAssistant;
class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;

class AIAssistantDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit AIAssistantDock(AIAssistant* assistant, QWidget* parent = nullptr);

    void setSelectionText(const QString& text);
    void focusQuestion();

private slots:
    void onSend();
    void onAnswerReady(const QString& answer);
    void onRequestFailed(const QString& errorMessage);

private:
    AIAssistant* assistant = nullptr;
    QString selectedText;

    QLabel* lblStatus = nullptr;
    QPlainTextEdit* viewSelection = nullptr;
    QPlainTextEdit* viewChat = nullptr;
    QLineEdit* editQuestion = nullptr;
    QPushButton* btnSend = nullptr;

    bool busy = false;
    void setBusy(bool isBusy);
    void appendChat(const QString& who, const QString& text);
};




