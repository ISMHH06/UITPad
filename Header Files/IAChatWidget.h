#ifndef IACHATWIDGET_H
#define IACHATWIDGET_H

#include <QDockWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QRegularExpression>
#include "AIAssistant.h"
#include "Settings.h"

class IAChatWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit IAChatWidget(AIAssistant *assistant, QWidget *parent = nullptr);

    void addMessage(const QString &sender, const QString &message, bool isError = false);
    void addCodeMessage(const QString &code);
    void addErrorMessage(const QString &error);
    void applyTheme(Settings::AppTheme theme);

public slots:
    void onSendClicked();
    void onClearClicked();

signals:
    void codeInsertRequested(const QString &code);
    void codeCopyRequested(const QString &code);

private:
    void setupUI();
    void setupStyle();
    void processAIResponse(const QString &response);
    void scrollToBottom();
    void refreshMessages();

    AIAssistant *assistant;
    Settings::AppTheme currentTheme;

    // Widgets
    QWidget *mainWidget;
    QScrollArea *scrollArea;
    QWidget *messagesContainer;
    QVBoxLayout *messagesLayout;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QPushButton *clearButton;
};

#endif // IACHATWIDGET_H
