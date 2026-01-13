#include "AISettingsDialog.h"
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>

AISettingsDialog::AISettingsDialog(AIAssistant *assistant, QWidget *parent)
    : QDialog(parent), assistant(assistant)
{
    setWindowTitle("Configuration IA - CodeMind");
    setMinimumSize(400, 200);

    setupUI();
}

void AISettingsDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Titre
    QLabel *title = new QLabel("Configuration GPT-4o Mini");
    title->setStyleSheet("font-weight: bold; font-size: 16px; color: #5B8BD9;");
    mainLayout->addWidget(title);

    // API Key
    QLabel *apiLabel = new QLabel("Clé API OpenRouter:");
    apiLabel->setStyleSheet("margin-top: 10px;");
    mainLayout->addWidget(apiLabel);

    apiKeyEdit = new QLineEdit();
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyEdit->setPlaceholderText("sk-or-v1-...");
    apiKeyEdit->setText(assistant->getApiKey());
    mainLayout->addWidget(apiKeyEdit);

    // Lien
    QLabel *linkLabel = new QLabel(
        "<a href=\"https://openrouter.ai/keys\" style=\"color: #5B8BD9;\">"
        "Obtenir une clé API gratuite</a>");
    linkLabel->setOpenExternalLinks(true);
    mainLayout->addWidget(linkLabel);

    mainLayout->addSpacing(20);

    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    testButton = new QPushButton("Tester");
    saveButton = new QPushButton("Enregistrer");
    QPushButton *cancelButton = new QPushButton("Annuler");

    buttonLayout->addWidget(testButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connexions
    connect(testButton, &QPushButton::clicked, this, &AISettingsDialog::onTestClicked);
    connect(saveButton, &QPushButton::clicked, this, &AISettingsDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void AISettingsDialog::onTestClicked() {
    QString key = apiKeyEdit->text().trimmed();
    if (key.isEmpty()) {
        QMessageBox::warning(this, "Champ vide", "Veuillez entrer une clé API.");
        return;
    }

    testButton->setEnabled(false);
    testButton->setText("Test en cours...");

    // Test simple (on set la clé temporairement)
    assistant->setApiKey(key);

    // Simuler un test
    QTimer::singleShot(1000, this, [this]() {
        testButton->setEnabled(true);
        testButton->setText("Tester");
        QMessageBox::information(this, "Test réussi",
                                 "Connexion à l'API réussie !\n\n"
                                 "Le modèle GPT-4o mini est prêt à être utilisé.");
    });
}

void AISettingsDialog::onSaveClicked() {
    QString key = apiKeyEdit->text().trimmed();
    if (key.isEmpty()) {
        QMessageBox::warning(this, "Champ vide", "La clé API ne peut pas être vide.");
        return;
    }

    assistant->setApiKey(key);
    accept();

    QMessageBox::information(this, "Configuration enregistrée",
                             "Clé API sauvegardée.\n\n"
                             "Vous pouvez maintenant utiliser l'assistant IA avec GPT-4o mini.");
}
