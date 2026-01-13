#ifndef AISETTINGSDIALOG_H
#define AISETTINGSDIALOG_H

#include <QDialog>
#include "AIAssistant.h"

class QLineEdit;
class QPushButton;

class AISettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AISettingsDialog(AIAssistant *assistant, QWidget *parent = nullptr);

private slots:
    void onSaveClicked();
    void onTestClicked();

private:
    void setupUI();

    AIAssistant *assistant;
    QLineEdit *apiKeyEdit;
    QPushButton *testButton;
    QPushButton *saveButton;
};

#endif // AISETTINGSDIALOG_H
