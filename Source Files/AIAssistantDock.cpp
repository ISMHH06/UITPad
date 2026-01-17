#include "AIAssistantDock.h"

#include "AIAssistant.h"
#include "Settings.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

AIAssistantDock::AIAssistantDock(AIAssistant* assistant_, QWidget* parent)
    : QDockWidget(parent),
      assistant(assistant_)
{
    setWindowTitle("AI Assistant");
    setObjectName("AIAssistantDock");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    QWidget* root = new QWidget(this);

    lblStatus = new QLabel(root);
    lblStatus->setWordWrap(true);
    lblStatus->setText("Select text, then ask a question.");

    viewSelection = new QPlainTextEdit(root);
    viewSelection->setReadOnly(true);
    viewSelection->setPlaceholderText("Selected text will appear here…");
    viewSelection->setMaximumBlockCount(2000);

    viewChat = new QPlainTextEdit(root);
    viewChat->setReadOnly(true);
    viewChat->setPlaceholderText("Chat history…");
    viewChat->setMaximumBlockCount(5000);

    editQuestion = new QLineEdit(root);
    editQuestion->setPlaceholderText("Ask anything…");

    btnSend = new QPushButton("Send", root);

    QHBoxLayout* askRow = new QHBoxLayout();
    askRow->addWidget(editQuestion, 1);
    askRow->addWidget(btnSend, 0);

    QVBoxLayout* layout = new QVBoxLayout(root);
    layout->addWidget(lblStatus);
    layout->addWidget(new QLabel("Selection:", root));
    layout->addWidget(viewSelection, 1);
    layout->addWidget(new QLabel("Chat:", root));
    layout->addWidget(viewChat, 3);
    layout->addLayout(askRow);

    root->setLayout(layout);
    setWidget(root);

    connect(btnSend, &QPushButton::clicked, this, &AIAssistantDock::onSend);
    connect(editQuestion, &QLineEdit::returnPressed, this, &AIAssistantDock::onSend);

    if (assistant) {
        connect(assistant, &AIAssistant::answerReady, this, &AIAssistantDock::onAnswerReady);
        connect(assistant, &AIAssistant::requestFailed, this, &AIAssistantDock::onRequestFailed);
    }
}

void AIAssistantDock::setSelectionText(const QString& text)
{
    selectedText = text;
    viewSelection->setPlainText(selectedText);
}

void AIAssistantDock::focusQuestion()
{
    editQuestion->setFocus();
    editQuestion->selectAll();
}

void AIAssistantDock::setBusy(bool isBusy)
{
    busy = isBusy;
    btnSend->setEnabled(!busy);
    editQuestion->setEnabled(!busy);
    if (busy) {
        lblStatus->setText("Thinking…");
    }
}

void AIAssistantDock::appendChat(const QString& who, const QString& text)
{
    QString t = text;
    t.replace("\r\n", "\n");
    viewChat->appendPlainText(QString("[%1]\n%2\n").arg(who, t));
}

void AIAssistantDock::onSend()
{
    if (!assistant) return;
    if (busy) return;

    const QString apiKey = Settings::deepSeekApiKey().trimmed(); // stored under ai/apiKey
    if (apiKey.isEmpty()) {
        lblStatus->setText("Missing API key. Please set it in Settings → IA → OpenRouter API key.");
        return;
    }

    const QString question = editQuestion->text().trimmed();
    if (question.isEmpty()) {
        lblStatus->setText("Please enter a question.");
        return;
    }

    appendChat("You", question);
    editQuestion->clear();
    setBusy(true);

    assistant->askDeepSeek(apiKey, question, selectedText, Settings::aiModel());
}

void AIAssistantDock::onAnswerReady(const QString& answer)
{
    setBusy(false);
    lblStatus->setText("Done.");
    appendChat("AI", answer);
}

void AIAssistantDock::onRequestFailed(const QString& errorMessage)
{
    setBusy(false);
    lblStatus->setText(errorMessage);
    appendChat("Error", errorMessage);
}




