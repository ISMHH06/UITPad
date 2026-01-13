#include "IAChatWidget.h"
#include <QScrollBar>
#include <QTimer>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QFont>
#include <QFontMetrics>
#include <QRegularExpression>

// CONSTRUCTEUR PRINCIPAL
IAChatWidget::IAChatWidget(AIAssistant *assistant, QWidget *parent)
    : QDockWidget("💬 Assistant IA", parent), assistant(assistant), currentTheme(Settings::Light)
{
    setupUI();
    setupStyle();

    connect(assistant, &AIAssistant::responseReady, this, &IAChatWidget::processAIResponse);
    connect(assistant, &AIAssistant::errorOccurred, this, &IAChatWidget::addErrorMessage);
}

void IAChatWidget::setupUI() {
    mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // Zone de messages avec défilement
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    messagesContainer = new QWidget();
    messagesLayout = new QVBoxLayout(messagesContainer);
    messagesLayout->setContentsMargins(10, 10, 10, 10);
    messagesLayout->setSpacing(15); // Plus d'espace entre les messages
    messagesLayout->addStretch();

    scrollArea->setWidget(messagesContainer);
    mainLayout->addWidget(scrollArea, 1);

    // Zone de saisie en bas
    QWidget *inputWidget = new QWidget();
    inputWidget->setFixedHeight(50);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(5);

    messageInput = new QLineEdit();
    messageInput->setPlaceholderText("Posez votre question... (// pour générer du code)");

    sendButton = new QPushButton("Envoyer");
    clearButton = new QPushButton("Effacer");

    inputLayout->addWidget(messageInput, 1);
    inputLayout->addWidget(sendButton);
    inputLayout->addWidget(clearButton);

    mainLayout->addWidget(inputWidget);

    setWidget(mainWidget);

    // Connexions
    connect(sendButton, &QPushButton::clicked, this, &IAChatWidget::onSendClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &IAChatWidget::onSendClicked);
    connect(clearButton, &QPushButton::clicked, this, &IAChatWidget::onClearClicked);

    // Message de bienvenue
    addMessage("IA", "👋 Bonjour ! Je suis CodeMind AI. Je peux :\n"
                     "• Expliquer du code (Ctrl+Shift+E)\n"
                     "• Compléter du code (Ctrl+Space)\n"
                     "• Générer du code depuis // commentaire\n"
                     "• Répondre à vos questions !");
}

void IAChatWidget::setupStyle() {
    // Style global du dock widget
    setStyleSheet(R"(
        QDockWidget {
            background: #709CA7;  //original #f8f9fa
            border: 1px solid #dee2e6;
            border-radius: 8px;
        }

        QDockWidget::title {
            background: linear-gradient(to right, #667eea, #764ba2);
            color: white;
            padding: 8px 15px;
            font-weight: bold;
            font-size: 13px;
            border-radius: 6px 6px 0 0;
        }

        QScrollArea {
            background: #ffffff;
            border: none;
            border-radius: 6px;
        }

        QLineEdit {
            background: white;
            border: 2px solid #e2e8f0;
            border-radius: 20px;
            padding: 8px 15px;
            font-size: 13px;
            selection-background-color: #667eea;
        }

        QLineEdit:focus {
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }

        QPushButton {
            background: linear-gradient(to right, #667eea, #764ba2);
            color: white;
            border: none;
            border-radius: 20px;
            padding: 8px 20px;
            font-weight: 600;
            font-size: 13px;
            min-width: 80px;
        }

        QPushButton:hover {
            background: linear-gradient(to right, #5a6fd8, #6a4190);
        }

        QPushButton:pressed {
            background: linear-gradient(to right, #4a5fc7, #5a3380);
        }

        QPushButton#clearBtn {
            background: #a0aec0;
        }

        QPushButton#clearBtn:hover {
            background: #90a0b0;
        }
    )");

    clearButton->setObjectName("clearBtn");
}

//  MÉTHODE : Traitement des réponses IA
void IAChatWidget::processAIResponse(const QString &response) {
    // Recherche de blocs de code
    QRegularExpression codeBlockRegex(R"(```(?:\w*\n)?([\s\S]*?)```)");

    QRegularExpressionMatchIterator it = codeBlockRegex.globalMatch(response);

    int lastPos = 0;
    bool hasCode = false;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        hasCode = true;

        // Texte avant le code
        QString before = response.mid(lastPos, match.capturedStart() - lastPos).trimmed();
        if (!before.isEmpty()) {
            addMessage("IA", before);
        }

        // Le code lui-même
        QString code = match.captured(1).trimmed();
        if (!code.isEmpty()) {
            addCodeMessage(code);
        }

        lastPos = match.capturedEnd();
    }

    // Texte après le dernier bloc de code
    QString after = response.mid(lastPos).trimmed();
    if (!after.isEmpty()) {
        addMessage("IA", after);
    }

    // Si pas de code du tout, afficher tout le message
    if (!hasCode) {
        addMessage("IA", response);
    }
}

// MÉTHODE POUR AJOUTER UN MESSAGE
void IAChatWidget::addMessage(const QString &sender, const QString &message, bool isError) {
    QWidget *messageWidget = new QWidget();
    messageWidget->setProperty("sender", sender);
    messageWidget->setProperty("isError", isError);
    messageWidget->setProperty("messageText", message);

    QHBoxLayout *layout = new QHBoxLayout(messageWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    // Avatar
    QLabel *avatar = new QLabel();
    QString avatarText;
    QString avatarStyle;

    if (sender == "Vous") {
        avatarText = "👤";
        avatarStyle = "background: #667eea;"; // Violet
    } else if (isError) {
        avatarText = "⚠";
        avatarStyle = "background: #f56565;"; // Rouge
    } else {
        avatarText = "🤖";
        avatarStyle = "background: #48bb78;"; // Vert
    }

    avatar->setText(avatarText);
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        avatarStyle +
        "color: white;"
        "border-radius: 18px;"
        "font-size: 16px;"
        "font-weight: bold;"
        );

    // Conteneur du message avec bord arrondi
    QWidget *msgContainer = new QWidget();
    msgContainer->setMinimumWidth(100);
    msgContainer->setMaximumWidth(450);

    QVBoxLayout *msgLayout = new QVBoxLayout(msgContainer);
    msgLayout->setContentsMargins(15, 12, 15, 12);
    msgLayout->setSpacing(5);

    // Texte du message
    QLabel *textLabel = new QLabel(message);
    textLabel->setWordWrap(true);
    textLabel->setTextFormat(Qt::PlainText);
    textLabel->setAlignment(Qt::AlignLeft);
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Style basé sur l'expéditeur et le thème
    QString msgStyle;
    QString textColor;

    if (sender == "Vous") {
        msgStyle = R"(
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:0,
                stop:0 #8a2be2,
                stop:0.5 #9b30ff,
                stop:1 #9370db
            );
            border-radius: 18px 18px 5px 18px;
            border: 1px solid rgba(138, 43, 226, 0.3);
        )";
        textColor = "color: white;";
    } else if (isError) {
        msgStyle = R"(
            background: linear-gradient(135deg, #f56565 0%, #ed64a6 100%);
            border-radius: 18px 18px 18px 5px;
        )";
        textColor = "color: white;";
    } else {
        // AI message - theme dependent
        if (currentTheme == Settings::Dark) {
            msgStyle = R"(
                background: #3C3C3C;
                border: 1px solid #4A4A4A;
                border-radius: 18px 18px 18px 5px;
            )";
            textColor = "color: #D4D4D4;";
        } else if (currentTheme == Settings::Hacker) {
            msgStyle = R"(
                background: #001100;
                border: 1px solid #00FF00;
                border-radius: 18px 18px 18px 5px;
            )";
            textColor = "color: #00FF00;";
        } else {
            // Light theme - improved visibility
            msgStyle = R"(
                background: #FFFFFF;
                border: 1px solid #CCCCCC;
                border-radius: 18px 18px 18px 5px;
            )";
            textColor = "color: #1a1a1a;";
        }
    }

    msgContainer->setStyleSheet(msgStyle);
    textLabel->setStyleSheet(textColor + "font-size: 13px; font-family: 'Segoe UI', Arial, sans-serif;");

    msgLayout->addWidget(textLabel);

    // Ajuster la hauteur du message
    QFontMetrics metrics(textLabel->font());
    int textWidth = metrics.horizontalAdvance(message);
    int lines = qMax(1, (textWidth / 400) + 1);
    int lineHeight = metrics.lineSpacing();
    msgContainer->setMinimumHeight(lines * lineHeight + 30);

    // Positionnement
    if (sender == "Vous") {
        layout->addStretch();
        layout->addWidget(msgContainer, 0, Qt::AlignRight);
        layout->addWidget(avatar);
        avatar->setStyleSheet(avatarStyle.replace(";", "; margin-left: 5px;"));
    } else {
        layout->addWidget(avatar);
        layout->addWidget(msgContainer, 0, Qt::AlignLeft);
        layout->addStretch();
        avatar->setStyleSheet(avatarStyle.replace(";", "; margin-right: 5px;"));
    }

    messagesLayout->insertWidget(messagesLayout->count() - 1, messageWidget);
    scrollToBottom();
}

// === MÉTHODE POUR AJOUTER UN MESSAGE DE CODE ===
void IAChatWidget::addCodeMessage(const QString &code) {
    QWidget *codeWidget = new QWidget();
    codeWidget->setProperty("isCodeMessage", true);
    codeWidget->setProperty("codeText", code);
    
    QVBoxLayout *layout = new QVBoxLayout(codeWidget);
    layout->setContentsMargins(0, 5, 0, 5);
    layout->setSpacing(8);

    // Conteneur du code avec style moderne
    QWidget *codeContainer = new QWidget();
    codeContainer->setStyleSheet(R"(
        background: #1a202c;
        border: 1px solid #2d3748;
        border-radius: 12px;
        padding: 15px;
    )");

    QVBoxLayout *containerLayout = new QVBoxLayout(codeContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(8);

    // En-tête du bloc de code
    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *langLabel = new QLabel("C++");
    langLabel->setStyleSheet(R"(
        background: #4fd1c7;
        color: #1a202c;
        font-weight: bold;
        padding: 3px 12px;
        border-radius: 10px;
        font-size: 10px;
        letter-spacing: 0.5px;
    )");

    QLabel *codeLabel = new QLabel("Code généré");
    codeLabel->setStyleSheet("color: #a0aec0; font-size: 11px; font-weight: 600;");

    headerLayout->addWidget(langLabel);
    headerLayout->addWidget(codeLabel);
    headerLayout->addStretch();

    // Zone de code
    QTextEdit *codeDisplay = new QTextEdit();
    codeDisplay->setPlainText(code);
    codeDisplay->setReadOnly(true);
    codeDisplay->setFont(QFont("Consolas", 10));
    codeDisplay->setLineWrapMode(QTextEdit::NoWrap);
    codeDisplay->setStyleSheet(R"(
        background: transparent;
        color: #e2e8f0;
        border: none;
        font-family: 'JetBrains Mono', 'Cascadia Code', Consolas, monospace;
        padding: 0;
        selection-background-color: #4a5568;
    )");

    // Ajuster la hauteur
    int lineCount = code.count('\n') + 1;
    int codeHeight = qMin(lineCount * 20 + 20, 300);
    codeDisplay->setFixedHeight(codeHeight);

    // Indentation fixe
    QFontMetrics metrics(codeDisplay->font());
    codeDisplay->setTabStopDistance(4 * metrics.horizontalAdvance(' '));

    // Barre d'outils pour les actions
    QWidget *toolbarWidget = new QWidget();
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(0, 5, 0, 0);

    QPushButton *copyBtn = new QPushButton("📋 Copier");
    QPushButton *insertBtn = new QPushButton("📝 Insérer");

    // Définir le style des boutons une fois pour toutes
    const QString btnStyle = R"(
        QPushButton {
            background: rgba(255, 255, 255, 0.1);
            color: #cbd5e0;
            border: 1px solid #4a5568;
            border-radius: 6px;
            padding: 6px 12px;
            font-weight: 500;
            font-size: 11px;
            margin-right: 8px;
        }
        QPushButton:hover {
            background: rgba(255, 255, 255, 0.15);
            border-color: #667eea;
        }
        QPushButton:pressed {
            background: rgba(255, 255, 255, 0.2);
        }
    )";

    const QString btnSuccessStyle = R"(
        QPushButton {
            background: rgba(255, 255, 255, 0.1);
            color: #48bb78;
            border: 1px solid #48bb78;
            border-radius: 6px;
            padding: 6px 12px;
            font-weight: 500;
            font-size: 11px;
            margin-right: 8px;
        }
        QPushButton:hover {
            background: rgba(255, 255, 255, 0.15);
            border-color: #48bb78;
        }
        QPushButton:pressed {
            background: rgba(255, 255, 255, 0.2);
        }
    )";

    copyBtn->setStyleSheet(btnStyle);
    insertBtn->setStyleSheet(btnStyle);

    toolbarLayout->addWidget(copyBtn);
    toolbarLayout->addWidget(insertBtn);
    toolbarLayout->addStretch();

    containerLayout->addWidget(headerWidget);
    containerLayout->addWidget(codeDisplay);
    containerLayout->addWidget(toolbarWidget);

    layout->addWidget(codeContainer);

    // Connexions des boutons - CORRIGÉ : Capture de toutes les variables nécessaires
    connect(copyBtn, &QPushButton::clicked, this, [this, copyBtn, code, btnStyle, btnSuccessStyle]() {
        QApplication::clipboard()->setText(code);
        copyBtn->setText("✓ Copié !");
        copyBtn->setStyleSheet(btnSuccessStyle);

        QTimer::singleShot(1500, copyBtn, [copyBtn, btnStyle]() {
            if (copyBtn) {
                copyBtn->setText("📋 Copier");
                copyBtn->setStyleSheet(btnStyle);
            }
        });

        emit codeCopyRequested(code);
    });

    connect(insertBtn, &QPushButton::clicked, this, [this, insertBtn, code, btnStyle, btnSuccessStyle]() {
        insertBtn->setText("✓ Inséré !");
        insertBtn->setStyleSheet(btnSuccessStyle);

        QTimer::singleShot(1500, insertBtn, [insertBtn, btnStyle]() {
            if (insertBtn) {
                insertBtn->setText("📝 Insérer");
                insertBtn->setStyleSheet(btnStyle);
            }
        });

        emit codeInsertRequested(code);
    });

    // Avatar pour le message de code
    QWidget *messageWidget = new QWidget();
    messageWidget->setProperty("isCodeMessage", true);
    messageWidget->setProperty("codeText", code);
    
    QHBoxLayout *messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    messageLayout->setSpacing(10);

    QLabel *avatar = new QLabel("🤖");
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        "background: #48bb78;"
        "color: white;"
        "border-radius: 18px;"
        "font-size: 16px;"
        "font-weight: bold;"
        "margin-right: 5px;"
        );

    messageLayout->addWidget(avatar);
    messageLayout->addWidget(codeWidget, 0, Qt::AlignLeft);
    messageLayout->addStretch();

    messagesLayout->insertWidget(messagesLayout->count() - 1, messageWidget);
    scrollToBottom();
}

void IAChatWidget::addErrorMessage(const QString &error) {
    addMessage("Erreur", error, true);
}

void IAChatWidget::onSendClicked() {
    QString text = messageInput->text().trimmed();
    if (text.isEmpty()) return;

    addMessage("Vous", text);
    messageInput->clear();

    if (text.startsWith("//")) {
        assistant->generateCode(text.mid(2));
    } else {
        assistant->askQuestion(text);
    }
}

void IAChatWidget::onClearClicked() {
    QLayoutItem *item;
    while ((item = messagesLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Réajouter le stretch
    messagesLayout->addStretch();

    // Message de bienvenue
    addMessage("IA", "Chat effacé. Posez-moi une question !");
}

void IAChatWidget::scrollToBottom() {
    QTimer::singleShot(50, this, [this]() {
        QScrollBar *scrollBar = scrollArea->verticalScrollBar();
        if (scrollBar) {
            scrollBar->setValue(scrollBar->maximum());
        }
    });
}

// Refresh all existing messages with current theme
void IAChatWidget::refreshMessages()
{
    // Collect all message data
    QList<QPair<QString, QString>> textMessages;  // sender, message
    QList<bool> errorFlags;
    QList<QString> codeMessages;
    
    // Traverse and collect message data
    for (int i = 0; i < messagesLayout->count(); ++i) {
        QLayoutItem* item = messagesLayout->itemAt(i);
        if (!item || !item->widget()) continue;
        
        QWidget* widget = item->widget();
        
        // Check if it's a code message
        if (widget->property("isCodeMessage").toBool()) {
            QString code = widget->property("codeText").toString();
            if (!code.isEmpty()) {
                codeMessages.append(code);
            }
        } else if (widget->property("sender").isValid()) {
            QString sender = widget->property("sender").toString();
            QString message = widget->property("messageText").toString();
            bool isError = widget->property("isError").toBool();
            
            if (!message.isEmpty()) {
                textMessages.append(qMakePair(sender, message));
                errorFlags.append(isError);
            }
        }
    }
    
    // Clear all messages
    QLayoutItem *item;
    while ((item = messagesLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // Re-add stretch
    messagesLayout->addStretch();
    
    // Re-add all messages with new theme
    int codeIdx = 0;
    for (int i = 0; i < textMessages.size(); ++i) {
        addMessage(textMessages[i].first, textMessages[i].second, errorFlags[i]);
    }
    
    for (const QString& code : codeMessages) {
        addCodeMessage(code);
    }
}

// Add this method to IAChatWidget.cpp
void IAChatWidget::applyTheme(Settings::AppTheme theme)
{
    currentTheme = theme;
    QString style;
    
    if (theme == Settings::Dark) {
        style = R"(
            QDockWidget {
                background: #252526;
                border: 1px solid #3E3E42;
            }
            QDockWidget::title {
                background: #2D2D30;
                color: #D4D4D4;
                padding: 8px 15px;
                font-weight: bold;
            }
            QScrollArea {
                background: #1E1E1E;
                border: none;
            }
            QWidget {
                background: #1E1E1E;
            }
            QLineEdit {
                background: #3C3C3C;
                color: #D4D4D4;
                border: 2px solid #3E3E42;
                border-radius: 20px;
                padding: 8px 15px;
            }
            QLineEdit:focus {
                border-color: #007ACC;
            }
            QPushButton {
                background: #007ACC;
                color: white;
                border: none;
                border-radius: 20px;
                padding: 8px 20px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: #1E90FF;
            }
            QPushButton#clearBtn {
                background: #4A4A4A;
            }
            QPushButton#clearBtn:hover {
                background: #5A5A5A;
            }
        )";
    }
    else if (theme == Settings::Hacker) {
        style = R"(
            QDockWidget {
                background: #000000;
                border: 1px solid #00FF00;
            }
            QDockWidget::title {
                background: #001100;
                color: #00FF00;
                padding: 8px 15px;
                font-weight: bold;
                font-family: 'Courier New', monospace;
            }
            QScrollArea {
                background: #000000;
                border: none;
            }
            QWidget {
                background: #000000;
            }
            QLineEdit {
                background: #001100;
                color: #00FF00;
                border: 2px solid #00FF00;
                border-radius: 20px;
                padding: 8px 15px;
                font-family: 'Courier New', monospace;
            }
            QPushButton {
                background: #003300;
                color: #00FF00;
                border: 1px solid #00FF00;
                border-radius: 20px;
                padding: 8px 20px;
                font-weight: 600;
                font-family: 'Courier New', monospace;
            }
            QPushButton:hover {
                background: #004400;
            }
        )";
    }
    else {
        // Light theme - improved visibility with dark text
        style = R"(
            QDockWidget {
                background: #f8f9fa;
                border: 1px solid #dee2e6;
                border-radius: 8px;
            }
            QDockWidget::title {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #667eea, stop:1 #764ba2);
                color: white;
                padding: 8px 15px;
                font-weight: bold;
            }
            QScrollArea {
                background: #F5F5F5;
                border: none;
            }
            QWidget {
                background: #F5F5F5;
            }
            QLineEdit {
                background: white;
                color: #1a1a1a;
                border: 2px solid #CCCCCC;
                border-radius: 20px;
                padding: 8px 15px;
            }
            QLineEdit:focus {
                border-color: #667eea;
            }
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #667eea, stop:1 #764ba2);
                color: white;
                border: none;
                bord    er-radius: 20px;
                padding: 8px 20px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #5a6fd8, stop:1 #6a4190);
            }
            QPushButton#clearBtn {
                background: #a0aec0;
            }
        )";
    }
    
    setStyleSheet(style);
    
    // Refresh existing messages to apply new theme colors
    refreshMessages();
}
