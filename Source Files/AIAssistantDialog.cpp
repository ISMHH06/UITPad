#include "AIAssistantDialog.h"

#include "AIAssistant.h"
#include "Settings.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

AIAssistantDialog::AIAssistantDialog(
    AIAssistant* assistant_,
    const QString& selectedText_,
    QWidget* parent
)
    : QDialog(parent),
    assistant(assistant_),
    selectedText(selectedText_)
{
    setWindowTitle("AI Assistant (OpenRouter)");
    setMinimumSize(520, 420);

    lblStatus = new QLabel(this);
    lblStatus->setText("Ask a question about the selected text (or anything).");
    lblStatus->setWordWrap(true);

    editSelection = new QPlainTextEdit(this);
    editSelection->setReadOnly(true);
    editSelection->setPlainText(selectedText);
    editSelection->setPlaceholderText("No selection.");
    editSelection->setMaximumBlockCount(2000);

    editQuestion = new QLineEdit(this);
    editQuestion->setPlaceholderText("Type your question…");

    btnSend = new QPushButton("Send", this);

    editAnswer = new QPlainTextEdit(this);
    editAnswer->setReadOnly(true);
    editAnswer->setPlaceholderText("Answer will appear here…");
    editAnswer->setMaximumBlockCount(5000);

    QHBoxLayout* askRow = new QHBoxLayout();
    askRow->addWidget(editQuestion, 1);
    askRow->addWidget(btnSend, 0);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(lblStatus);
    layout->addWidget(new QLabel("Selected text:", this));
    layout->addWidget(editSelection, 1);
    layout->addLayout(askRow);
    layout->addWidget(new QLabel("Answer:", this));
    layout->addWidget(editAnswer, 2);
    setLayout(layout);

    connect(btnSend, &QPushButton::clicked, this, &AIAssistantDialog::onSend);
    connect(editQuestion, &QLineEdit::returnPressed, this, &AIAssistantDialog::onSend);

    if (assistant) {
        connect(assistant, &AIAssistant::answerReady, this, &AIAssistantDialog::onAnswerReady);
        connect(assistant, &AIAssistant::requestFailed, this, &AIAssistantDialog::onRequestFailed);
    }
}

void AIAssistantDialog::setBusy(bool busy)
{
    btnSend->setEnabled(!busy);
    editQuestion->setEnabled(!busy);
    if (busy) {
        lblStatus->setText("Thinking…");
    }
}

void AIAssistantDialog::onSend()
{
    if (!assistant) return;

    QString apiKey = Settings::deepSeekApiKey().trimmed();
    if (apiKey.isEmpty()) {
        lblStatus->setText("Missing API key. Please set it in Settings → IA → OpenRouter API key.");
        return;
    }

    const QString question = editQuestion->text().trimmed();
    if (question.isEmpty()) {
        lblStatus->setText("Please enter a question.");
        return;
    }

    setBusy(true);
    editAnswer->setPlainText("...");
    assistant->askDeepSeek(apiKey, question, selectedText, Settings::aiModel());
}

void AIAssistantDialog::onAnswerReady(const QString& answer)
{
    setBusy(false);
    lblStatus->setText("Done.");
    editAnswer->setPlainText(answer);
}

void AIAssistantDialog::onRequestFailed(const QString& errorMessage)
{
    setBusy(false);
    lblStatus->setText(errorMessage);
    editAnswer->setPlainText(errorMessage);
}



