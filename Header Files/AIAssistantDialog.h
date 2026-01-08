#pragma once

#include <QDialog>
#include <QString>

class AIAssistant;
class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;

class AIAssistantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AIAssistantDialog(
        AIAssistant* assistant,
        const QString& selectedText,
        QWidget* parent = nullptr
    );

private slots:
    void onSend();
    void onAnswerReady(const QString& answer);
    void onRequestFailed(const QString& errorMessage);

private:
    AIAssistant* assistant = nullptr;
    QString selectedText;

    QLabel* lblStatus = nullptr;
    QPlainTextEdit* editSelection = nullptr;
    QLineEdit* editQuestion = nullptr;
    QPushButton* btnSend = nullptr;
    QPlainTextEdit* editAnswer = nullptr;

    void setBusy(bool busy);
};



