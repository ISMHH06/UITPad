# AI Assistant Feature - Technical Explanation

## Overview
The UITPad text editor includes an integrated AI Assistant feature that allows users to get AI-powered help directly within the application. Users can select text in their document and ask questions about it, or ask general questions, receiving intelligent responses from an AI model.

**Technology Stack**: C++17, Qt6/Qt5 Framework, OpenRouter REST API, JSON, HTTP/HTTPS

---

## What It Does

### User Functionality
1. **Context-Aware Assistance**: Users can select text in their document and ask questions about it
2. **General Questions**: Users can ask any question without selecting text
3. **Interactive Chat**: The AI responses appear in a dockable panel that can be moved around the interface
4. **Chat History**: The dock maintains a conversation history showing both user questions and AI responses

### Example Use Cases
- **Code Explanation**: Select a code snippet and ask "What does this code do?"
- **Code Debugging**: Select problematic code and ask "Why is this not working?"
- **Code Improvement**: Ask "How can I optimize this function?"
- **General Questions**: Ask programming questions, syntax help, or conceptual explanations

---

## Technical Architecture

### System Architecture Diagram
```
┌─────────────────────────────────────────────────────────────┐
│                      MainWindow (QMainWindow)                │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         QTabWidget (Document Management)             │  │
│  │  ┌────────────────────────────────────────────────┐  │  │
│  │  │     TextEditor (QPlainTextEdit)                │  │  │
│  │  │     - Text selection handling                  │  │  │
│  │  │     - Selection change signals                 │  │  │
│  │  └────────────────────────────────────────────────┘  │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         AIAssistantDock (QDockWidget)                │  │
│  │  - Selection view (QPlainTextEdit)                   │  │
│  │  - Chat history (QPlainTextEdit)                     │  │
│  │  - Question input (QLineEdit)                        │  │
│  │  - Send button (QPushButton)                         │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ Signals/Slots
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              AIAssistant (QObject)                          │
│  - QNetworkAccessManager (member variable)                 │
│  - askDeepSeek() method                                     │
│  - Signals: answerReady(), requestFailed()                 │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ HTTP POST
                            ▼
┌─────────────────────────────────────────────────────────────┐
│         OpenRouter API (REST Endpoint)                      │
│  https://openrouter.ai/api/v1/chat/completions              │
│  - Bearer token authentication                              │
│  - JSON request/response                                    │
└─────────────────────────────────────────────────────────────┘
```

### Component Interaction Flow
```
User Action → UI Event → Signal Emission → Slot Execution
    ↓
AIAssistant::askDeepSeek()
    ↓
QNetworkAccessManager::post()
    ↓
HTTP Request (asynchronous)
    ↓
QNetworkReply (event-driven)
    ↓
Signal: finished()
    ↓
Lambda callback execution
    ↓
JSON parsing (QJsonDocument)
    ↓
Signal: answerReady() / requestFailed()
    ↓
UI update via connected slots
```

---

## Technical Architecture

### Components

#### 1. **AIAssistant Class** (`AIAssistant.h/cpp`)

**Class Declaration**:
```cpp
class AIAssistant : public QObject
{
    Q_OBJECT  // Required for Qt's Meta-Object Compiler (MOC)
    
public:
    explicit AIAssistant(QObject* parent = nullptr);
    void askDeepSeek(const QString& apiKey, const QString& question, 
                     const QString& selectedText, const QString& model = "deepseek-chat");
    
signals:
    void answerReady(const QString& answer);
    void requestFailed(const QString& errorMessage);
    
private:
    QNetworkAccessManager network;  // Member variable, not pointer (Qt ownership)
    static QString buildUserContent(const QString& question, const QString& selectedText);
};
```

**Technical Details**:
- **Inheritance**: Inherits from `QObject` to enable Qt's signal-slot mechanism and MOC processing
- **Q_OBJECT Macro**: Enables Qt's Meta-Object System, allowing runtime introspection, signals/slots, and property system
- **Network Manager**: `QNetworkAccessManager` is a member variable (not pointer) - Qt manages its lifecycle automatically
- **Thread Safety**: `QNetworkAccessManager` is thread-safe and can be used from any thread, but all slots connected to its signals must be in the same thread (main thread in this case)
- **Memory Management**: Network replies are managed via `QNetworkReply::deleteLater()` to ensure proper cleanup in Qt's event loop

**Key Methods**:

**`askDeepSeek()` Implementation Details**:
```cpp
void AIAssistant::askDeepSeek(const QString& apiKey, const QString& question,
                               const QString& selectedText, const QString& model)
{
    // Input validation
    const QString trimmedKey = apiKey.trimmed();
    if (trimmedKey.isEmpty()) {
        emit requestFailed("Missing DeepSeek API key...");
        return;  // Early return pattern
    }
    
    // URL construction
    QUrl url("https://openrouter.ai/api/v1/chat/completions");
    
    // HTTP Request setup
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Accept", "application/json");
    req.setRawHeader("Authorization", QString("Bearer %1").arg(trimmedKey).toUtf8());
    req.setRawHeader("HTTP-Referer", "https://uitpad.local");
    req.setRawHeader("X-Title", "UITPad");
    
    // JSON payload construction
    QJsonArray messages;
    messages.append(QJsonObject{
        { "role", "system" },
        { "content", "You are a helpful assistant..." }
    });
    messages.append(QJsonObject{
        { "role", "user" },
        { "content", buildUserContent(question, selectedText) }
    });
    
    QJsonObject payload{
        { "model", model },
        { "messages", messages },
        { "temperature", 0.2 },  // Low temperature = more deterministic
        { "stream", false }      // Non-streaming mode
    };
    
    // Asynchronous POST request
    QNetworkReply* reply = network.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    
    // Lambda-based callback (C++11/14 feature)
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Response handling logic...
        reply->deleteLater();  // Schedule for deletion in event loop
    });
}
```

**Technical Aspects**:
- **Lambda Capture**: `[this, reply]` captures `this` pointer and `reply` pointer by value
- **JSON Serialization**: `QJsonDocument::toJson(QJsonDocument::Compact)` creates compact JSON (no whitespace)
- **UTF-8 Encoding**: All strings are converted to UTF-8 via `toUtf8()` for HTTP headers
- **Asynchronous Pattern**: `network.post()` returns immediately, actual HTTP request happens asynchronously
- **Event Loop Integration**: `finished()` signal is emitted when request completes, processed by Qt's event loop

**`buildUserContent()` Static Method**:
```cpp
static QString buildUserContent(const QString& question, const QString& selectedText)
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
    return content;  // Return by value (QString uses implicit sharing)
}
```
- **Static Method**: No access to instance members, can be called without object instance
- **QString Implicit Sharing**: Return by value is efficient due to Qt's copy-on-write mechanism
- **String Concatenation**: Uses `operator+=` which is optimized in Qt's QString implementation

**Signals** (MOC-generated):
- `answerReady(const QString& answer)` - Emitted when AI response is successfully parsed
- `requestFailed(const QString& errorMessage)` - Emitted on any error condition
- **Signal Emission**: Thread-safe, can be emitted from any thread, but connected slots execute in receiver's thread

#### 2. **AIAssistantDock Class** (`AIAssistantDock.h/cpp`)

**Class Declaration**:
```cpp
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
    AIAssistant* assistant = nullptr;  // Raw pointer (parent manages lifecycle)
    QString selectedText;
    
    // UI Components (parented to dock widget)
    QLabel* lblStatus = nullptr;
    QPlainTextEdit* viewSelection = nullptr;
    QPlainTextEdit* viewChat = nullptr;
    QLineEdit* editQuestion = nullptr;
    QPushButton* btnSend = nullptr;
    
    bool busy = false;  // State flag for request in progress
    void setBusy(bool isBusy);
    void appendChat(const QString& who, const QString& text);
};
```

**Technical Details**:
- **Inheritance**: `QDockWidget` provides dockable window functionality in `QMainWindow`
- **Widget Hierarchy**: All child widgets are parented to the dock, ensuring automatic memory management
- **Layout System**: Uses `QVBoxLayout` and `QHBoxLayout` for automatic widget positioning and resizing
- **Dock Features**: Configured with `setAllowedAreas()` and `setFeatures()` for UI customization

**Constructor Implementation**:
```cpp
AIAssistantDock::AIAssistantDock(AIAssistant* assistant_, QWidget* parent)
    : QDockWidget(parent), assistant(assistant_)
{
    setWindowTitle("AI Assistant");
    setObjectName("AIAssistantDock");  // For QSettings persistence
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | 
                QDockWidget::DockWidgetFloatable | 
                QDockWidget::DockWidgetClosable);
    
    QWidget* root = new QWidget(this);  // Parented to dock
    
    // Widget creation with parent
    lblStatus = new QLabel(root);
    viewSelection = new QPlainTextEdit(root);
    viewSelection->setReadOnly(true);
    viewSelection->setMaximumBlockCount(2000);  // Memory limit
    
    viewChat = new QPlainTextEdit(root);
    viewChat->setReadOnly(true);
    viewChat->setMaximumBlockCount(5000);  // Chat history limit
    
    editQuestion = new QLineEdit(root);
    btnSend = new QPushButton("Send", root);
    
    // Layout construction
    QVBoxLayout* layout = new QVBoxLayout(root);
    layout->addWidget(lblStatus);
    layout->addWidget(new QLabel("Selection:", root));
    layout->addWidget(viewSelection, 1);  // Stretch factor = 1
    layout->addWidget(new QLabel("Chat:", root));
    layout->addWidget(viewChat, 3);  // Stretch factor = 3 (more space)
    layout->addLayout(askRow);
    
    // Signal-slot connections
    connect(btnSend, &QPushButton::clicked, this, &AIAssistantDock::onSend);
    connect(editQuestion, &QLineEdit::returnPressed, this, &AIAssistantDock::onSend);
    
    if (assistant) {
        connect(assistant, &AIAssistant::answerReady, 
                this, &AIAssistantDock::onAnswerReady);
        connect(assistant, &AIAssistant::requestFailed, 
                this, &AIAssistantDock::onRequestFailed);
    }
}
```

**Technical Aspects**:
- **Memory Management**: All widgets use Qt's parent-child ownership model - deletion is automatic
- **Layout Stretch Factors**: `addWidget(widget, stretch)` controls relative sizing in layout
- **Maximum Block Count**: `setMaximumBlockCount()` limits memory usage by removing old lines
- **Signal-Slot Syntax**: Uses new-style `&Class::method` syntax (type-safe, compile-time checked)
- **Lambda Alternative**: Could use lambdas, but member function pointers are more explicit

**State Management**:
```cpp
void AIAssistantDock::setBusy(bool isBusy)
{
    busy = isBusy;
    btnSend->setEnabled(!busy);      // Disable during request
    editQuestion->setEnabled(!busy); // Prevent multiple requests
    if (busy) {
        lblStatus->setText("Thinking…");
    }
}
```
- **Atomic State**: `busy` flag prevents concurrent requests
- **UI Feedback**: Immediate visual feedback via button/input disabling

**Chat History Management**:
```cpp
void AIAssistantDock::appendChat(const QString& who, const QString& text)
{
    QString t = text;
    t.replace("\r\n", "\n");  // Normalize line endings (Windows → Unix)
    viewChat->appendPlainText(QString("[%1]\n%2\n").arg(who, t));
}
```
- **Line Ending Normalization**: Handles Windows (`\r\n`) vs Unix (`\n`) line endings
- **String Formatting**: Uses `QString::arg()` for efficient string interpolation
- **Append vs Insert**: `appendPlainText()` is O(1) operation, more efficient than prepending

**Features**:
- **Dock Persistence**: `setObjectName()` allows QMainWindow to save/restore dock state
- **Focus Management**: `focusQuestion()` uses `setFocus()` and `selectAll()` for UX
- **Text Selection Sync**: `setSelectionText()` updates selection view when user selects text in editor

#### 3. **AIAssistantDialog Class** (`AIAssistantDialog.h/cpp`)
- **Purpose**: Alternative modal dialog interface for AI assistance
- **Difference from Dock**: Opens as a separate window instead of a dock panel
- **Use Case**: For users who prefer popup dialogs over persistent panels

---

## API Integration

### Service Provider: OpenRouter
- **URL**: `https://openrouter.ai/api/v1/chat/completions`
- **Protocol**: REST API (OpenAI-compatible), HTTPS (TLS 1.2+)
- **HTTP Method**: POST
- **Content-Type**: `application/json`
- **Authentication**: Bearer token in `Authorization` header
- **API Compatibility**: OpenAI Chat Completions API v1 format

### HTTP Request Construction

**URL and Headers**:
```cpp
QUrl url("https://openrouter.ai/api/v1/chat/completions");
QNetworkRequest req(url);

// Required headers
req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
req.setRawHeader("Accept", "application/json");
req.setRawHeader("Authorization", QString("Bearer %1").arg(trimmedKey).toUtf8());

// Optional headers (OpenRouter recommendation)
req.setRawHeader("HTTP-Referer", "https://uitpad.local");
req.setRawHeader("X-Title", "UITPad");
```

**Technical Details**:
- **QUrl**: Qt's URL class handles encoding, parsing, and validation
- **Raw Headers**: `setRawHeader()` sets custom HTTP headers (case-sensitive)
- **UTF-8 Encoding**: `toUtf8()` converts QString to QByteArray for HTTP transmission
- **Bearer Token**: OAuth 2.0 Bearer token authentication scheme

### Request Payload Construction

**JSON Structure**:
```cpp
QJsonArray messages;
messages.append(QJsonObject{
    { "role", "system" },
    { "content", "You are a helpful assistant. Answer clearly and concisely. "
                  "If code is involved, provide correct code and brief explanations." }
});
messages.append(QJsonObject{
    { "role", "user" },
    { "content", buildUserContent(question, selectedText) }
});

QJsonObject payload{
    { "model", model },           // Model identifier string
    { "messages", messages },      // Array of message objects
    { "temperature", 0.2 },       // Float: 0.0-2.0 (lower = more deterministic)
    { "stream", false }           // Boolean: false = non-streaming response
};
```

**JSON Serialization**:
```cpp
QByteArray jsonData = QJsonDocument(payload).toJson(QJsonDocument::Compact);
// Result: {"model":"deepseek-chat","messages":[...],"temperature":0.2,"stream":false}
```

**Technical Aspects**:
- **QJsonObject/QJsonArray**: Qt's JSON API (Qt 5.0+)
- **Initializer Lists**: C++11 brace initialization for JSON construction
- **Compact Format**: `QJsonDocument::Compact` removes whitespace (smaller payload)
- **Temperature Parameter**: Controls randomness (0.2 = focused, deterministic responses)
- **Streaming**: `false` means complete response in one HTTP response (simpler implementation)

### HTTP Request Execution

**Asynchronous POST**:
```cpp
QNetworkReply* reply = network.post(req, jsonData);
// Returns immediately, actual HTTP request happens asynchronously
```

**Technical Details**:
- **Non-Blocking**: `post()` returns `QNetworkReply*` immediately
- **Event-Driven**: Actual network I/O happens in Qt's event loop
- **Thread Safety**: `QNetworkAccessManager` is thread-safe, but reply must be handled in same thread
- **Connection Pooling**: Qt automatically manages HTTP connection reuse

### Response Handling

**Network Reply Processing**:
```cpp
connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    // Read response data
    const QByteArray raw = reply->readAll();
    
    // Get HTTP status code
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        // Handle network-level errors (DNS, connection, timeout)
        emit requestFailed(QString("Network error: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }
    
    // Check HTTP status codes
    if (statusCode >= 400) {
        // Handle HTTP-level errors
        if (statusCode == 401) {
            emit requestFailed("Unauthorized - Check API key");
        } else if (statusCode == 402) {
            emit requestFailed("Payment required - Insufficient credits");
        } else if (statusCode == 429) {
            emit requestFailed("Rate limited - Please wait");
        }
        reply->deleteLater();
        return;
    }
    
    // Parse JSON response
    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        emit requestFailed("Failed to parse JSON response");
        reply->deleteLater();
        return;
    }
    
    // Extract AI response
    const QJsonObject obj = doc.object();
    const QJsonArray choices = obj.value("choices").toArray();
    const QJsonObject firstChoice = choices.at(0).toObject();
    const QJsonObject message = firstChoice.value("message").toObject();
    const QString answer = message.value("content").toString().trimmed();
    
    emit answerReady(answer);
    reply->deleteLater();
});
```

**Response JSON Structure**:
```json
{
  "id": "chatcmpl-...",
  "object": "chat.completion",
  "created": 1234567890,
  "model": "deepseek-chat",
  "choices": [
    {
      "index": 0,
      "message": {
        "role": "assistant",
        "content": "The AI response text here..."
      },
      "finish_reason": "stop"
    }
  ],
  "usage": {
    "prompt_tokens": 100,
    "completion_tokens": 50,
    "total_tokens": 150
  }
}
```

**Technical Details**:
- **QByteArray**: Raw HTTP response body (binary data)
- **HTTP Status Codes**: Retrieved via `QNetworkRequest::HttpStatusCodeAttribute`
- **Error Categories**: Network errors vs HTTP errors (different handling)
- **JSON Parsing**: `QJsonDocument::fromJson()` with error reporting
- **Null Safety**: `toArray()`, `toObject()` return empty/null if conversion fails
- **String Extraction**: `toString().trimmed()` removes whitespace
- **Memory Cleanup**: `deleteLater()` schedules deletion in event loop (safe for signal handlers)

### Error Management

**Error Categories**:

1. **Network-Level Errors** (`QNetworkReply::error()`):
   - `QNetworkReply::ConnectionRefusedError` - Server not reachable
   - `QNetworkReply::RemoteHostClosedError` - Connection closed
   - `QNetworkReply::TimeoutError` - Request timeout
   - `QNetworkReply::SslHandshakeFailedError` - TLS/SSL failure
   - `QNetworkReply::NetworkAccessDeniedError` - Firewall/proxy blocking

2. **HTTP Status Codes**:
   - **200-299**: Success
   - **400**: Bad Request (malformed payload)
   - **401**: Unauthorized (invalid/missing API key)
   - **402**: Payment Required (insufficient credits)
   - **403**: Forbidden (API key lacks permissions)
   - **429**: Too Many Requests (rate limit exceeded)
   - **500-599**: Server errors

3. **JSON Parsing Errors** (`QJsonParseError`):
   - `QJsonParseError::NoError` - Success
   - `QJsonParseError::IllegalValue` - Invalid JSON syntax
   - `QJsonParseError::MissingObjectSeparator` - Missing comma
   - `QJsonParseError::MissingNameSeparator` - Missing colon
   - `QJsonParseError::UnterminatedObject` - Missing closing brace
   - `QJsonParseError::UnterminatedArray` - Missing closing bracket

4. **Application-Level Errors**:
   - Empty API key
   - Empty question
   - Empty AI response
   - Missing "choices" array in response
   - Missing "content" field in message

**Error Handling Strategy**:
- **Early Returns**: Validate inputs before making HTTP request
- **Graceful Degradation**: Show user-friendly error messages
- **Error Propagation**: Use signals to communicate errors to UI
- **Resource Cleanup**: Always call `reply->deleteLater()` in all code paths

---

## Integration with Main Application

### MainWindow Integration

**Initialization**:
```cpp
// In MainWindow constructor
aiAssistant = new AIAssistant(this);  // Parented to MainWindow
// aiDock is created lazily (on first use)
```

**Lazy Initialization Pattern**:
```cpp
AIAssistantDock* MainWindow::ensureAiDock()
{
    if (aiDock) return aiDock;  // Already created
    
    // Create on first access
    aiDock = new AIAssistantDock(aiAssistant, this);
    addDockWidget(Qt::RightDockWidgetArea, aiDock);
    aiDock->hide();  // Hidden by default
    return aiDock;
}
```

**Technical Benefits**:
- **Memory Efficiency**: Dock only created when needed
- **Startup Performance**: Faster application startup
- **Resource Management**: Parent-child ownership ensures cleanup

**Text Selection Integration**:
```cpp
void MainWindow::openAiDockForTextEdit(QPlainTextEdit* textEdit)
{
    AIAssistantDock* dock = ensureAiDock();
    
    // Get selected text from QTextCursor
    QString selected = normalizedQtSelection(textEdit->textCursor().selectedText());
    
    // Update dock with selection
    dock->setSelectionText(selected);
    
    // Show and focus dock
    dock->show();
    dock->raise();  // Bring to front
    dock->focusQuestion();  // Focus input field
}
```

**QTextCursor Details**:
- **Selection API**: `textCursor().selectedText()` returns selected text
- **Qt Special Characters**: Returns paragraph separators (U+2029) instead of `\n`
- **Normalization Required**: Must convert to standard newlines for API

**Menu Integration**:
```cpp
// Menu action (in MainWindow)
QAction* aiAction = menu->addAction("AI Assistant");
aiAction->setShortcut(QKeySequence("Ctrl+Shift+A"));
connect(aiAction, &QAction::triggered, this, [this]() {
    QPlainTextEdit* editor = getCurrentEditor();
    if (editor) {
        openAiDockForTextEdit(editor);
    }
});
```

### Settings Integration

**QSettings API**:
```cpp
// In Settings class
static QString deepSeekApiKey()
{
    QSettings settings;
    settings.beginGroup("ai");
    return settings.value("apiKey", "").toString();
}

static void setDeepSeekApiKey(const QString& apiKey)
{
    QSettings settings;
    settings.beginGroup("ai");
    settings.setValue("apiKey", apiKey);
    settings.endGroup();
    settings.sync();  // Flush to disk
}
```

**QSettings Storage**:
- **Windows**: Registry (`HKEY_CURRENT_USER\Software\<Organization>\<Application>`)
- **macOS**: Property list files (`~/Library/Preferences/`)
- **Linux**: INI files (`~/.config/<Organization>/<Application>.conf`)

**Settings Grouping**:
```cpp
settings.beginGroup("ai");  // Creates "ai/" prefix
settings.setValue("apiKey", key);      // Stored as "ai/apiKey"
settings.setValue("model", model);     // Stored as "ai/model"
settings.endGroup();
```

**Settings Persistence**:
- **Automatic**: QSettings writes to disk automatically
- **Sync**: `sync()` ensures immediate write (optional)
- **Thread Safety**: QSettings is thread-safe for reading, but not writing
- **Encryption**: Windows 10+ encrypts registry values automatically

**Model Selection**:
```cpp
static QString aiModel()
{
    QSettings settings;
    settings.beginGroup("ai");
    return settings.value("model", "deepseek-chat").toString();  // Default value
}
```

**Default Values**:
- **API Key**: Empty string (user must provide)
- **Model**: "deepseek-chat" (sensible default)
- **Fallback**: `value(key, defaultValue)` provides type-safe defaults

### Settings UI Integration

**Settings Dialog**:
```cpp
// In Settings constructor
editDeepSeekApiKey = new QLineEdit(this);
editDeepSeekApiKey->setEchoMode(QLineEdit::Password);  // Hide API key
editDeepSeekApiKey->setText(deepSeekApiKey());  // Load from settings

editAiModel = new QLineEdit(this);
editAiModel->setText(aiModel());  // Load from settings
```

**Security Considerations**:
- **Password Echo Mode**: API key hidden with `***` characters
- **No Logging**: API key never logged or printed
- **Memory**: API key stored in QString (cleared when object destroyed)

**Settings Application**:
```cpp
void Settings::onApply()
{
    setDeepSeekApiKey(editDeepSeekApiKey->text());
    setAiModel(editAiModel->text());
    saveToSettings();  // Persist to disk
    emit settingsChanged();  // Notify other components
}
```

**Settings Change Notification**:
- **Signal Emission**: `settingsChanged()` signal notifies listeners
- **Dynamic Updates**: Components can react to settings changes
- **No Restart Required**: Settings take effect immediately

---

## Technical Implementation Details

### Network Communication

**QNetworkAccessManager Architecture**:
- **Singleton Pattern**: One instance per application (or per thread)
- **Event Loop Integration**: Uses Qt's event loop for asynchronous I/O
- **Connection Pooling**: Automatically reuses HTTP connections (HTTP/1.1 keep-alive)
- **SSL/TLS Support**: Handles HTTPS automatically via QSslSocket
- **Proxy Support**: Respects system proxy settings
- **Cookie Management**: Automatic cookie handling (if needed)

**Asynchronous I/O Pattern**:
```cpp
// Synchronous (blocking) - NOT USED
QNetworkReply* reply = network.post(req, data);
reply->waitForReadyRead();  // BLOCKS UI THREAD - BAD

// Asynchronous (non-blocking) - USED
QNetworkReply* reply = network.post(req, data);
connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    // Executes when response arrives - UI remains responsive
});
```

**Threading Model**:
- **Main Thread**: All UI operations and network callbacks execute in main thread
- **Worker Threads**: Qt's network subsystem uses worker threads internally
- **Thread Safety**: `QNetworkAccessManager` is thread-safe, but `QNetworkReply` must be used in thread where created
- **Signal Emission**: Signals are thread-safe, but connected slots execute in receiver's thread

**Network Reply Lifecycle**:
```cpp
QNetworkReply* reply = network.post(req, data);
// 1. Reply object created (owned by QNetworkAccessManager)
// 2. HTTP request sent asynchronously
// 3. Data received in chunks (readyRead() signal)
// 4. finished() signal emitted when complete
// 5. reply->deleteLater() schedules deletion
// 6. Qt event loop deletes object when safe
```

### Text Processing

**Qt Text Selection Handling**:
```cpp
QString selected = textEdit->textCursor().selectedText();
// Qt uses special Unicode characters for line breaks:
// - Paragraph separator (U+2029) instead of \n
// - Line separator (U+2028) for soft breaks
```

**Text Normalization**:
```cpp
QString normalizedQtSelection(const QString& qtText)
{
    QString result = qtText;
    // Replace Qt's paragraph separator with standard newline
    result.replace(QChar(0x2029), QChar('\n'));
    // Replace Qt's line separator with standard newline
    result.replace(QChar(0x2028), QChar('\n'));
    return result;
}
```

**Context Building**:
- **String Concatenation**: Uses `QString::operator+=` (optimized with implicit sharing)
- **Memory Efficiency**: Qt's copy-on-write means string copies are cheap
- **Unicode Support**: Full UTF-8/UTF-16 support for international text
- **Empty Handling**: `trimmed().isEmpty()` check prevents unnecessary API calls

### UI State Management

**State Machine Pattern**:
```
IDLE → [User clicks Send] → BUSY → [Response received] → IDLE
                              ↓
                         [Error occurred] → ERROR → IDLE
```

**Busy State Implementation**:
```cpp
void AIAssistantDock::setBusy(bool isBusy)
{
    busy = isBusy;  // Atomic state flag
    
    // Disable UI controls
    btnSend->setEnabled(!busy);
    editQuestion->setEnabled(!busy);
    
    // Visual feedback
    if (busy) {
        lblStatus->setText("Thinking…");
        // Could add: progress bar, spinner, cursor change
    }
}
```

**Signal-Slot Connections**:
```cpp
// Type-safe connections (compile-time checked)
connect(assistant, &AIAssistant::answerReady, 
        this, &AIAssistantDock::onAnswerReady);

// vs Old-style (runtime checked, less safe)
connect(assistant, SIGNAL(answerReady(QString)), 
        this, SLOT(onAnswerReady(QString)));
```

**Qt's Meta-Object System**:
- **MOC (Meta-Object Compiler)**: Preprocessor generates meta-object code
- **Runtime Type Information**: `Q_OBJECT` macro enables `qobject_cast<>()`
- **Property System**: Can use `Q_PROPERTY` for settings (not used here)
- **Signal-Slot Mechanism**: Type-safe, decoupled communication

### Memory Management

**Qt's Ownership Model**:
```cpp
// Parent-child ownership
QWidget* parent = new QWidget();
QPushButton* child = new QPushButton(parent);
// When parent is deleted, child is automatically deleted

// vs Manual management
QPushButton* button = new QPushButton();
// Must manually: delete button;  // Memory leak if forgotten
```

**Widget Hierarchy in AIAssistantDock**:
```
AIAssistantDock (QDockWidget)
└── QWidget (root)
    ├── QLabel (lblStatus)
    ├── QLabel ("Selection:")
    ├── QPlainTextEdit (viewSelection)
    ├── QLabel ("Chat:")
    ├── QPlainTextEdit (viewChat)
    ├── QHBoxLayout (askRow)
    │   ├── QLineEdit (editQuestion)
    │   └── QPushButton (btnSend)
    └── QVBoxLayout (main layout)
```

**Network Reply Cleanup**:
```cpp
// CORRECT: deleteLater() (safe in signal handler)
connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    // Process reply...
    reply->deleteLater();  // Schedules deletion in event loop
});

// WRONG: Direct delete (unsafe in signal handler)
connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    delete reply;  // DANGEROUS: may delete object during signal emission
});
```

**Text Storage Optimization**:
```cpp
viewChat->setMaximumBlockCount(5000);
// When limit exceeded, oldest blocks are removed
// Prevents unbounded memory growth
// O(1) removal from front (Qt's internal optimization)
```

**QString Memory Management**:
- **Implicit Sharing**: Multiple QString objects can share same data
- **Copy-on-Write**: Data copied only when modified
- **Reference Counting**: Automatic memory management
- **Small String Optimization**: Strings < 24 bytes stored inline (no heap allocation)

### Event Loop Integration

**Qt's Event Loop**:
```cpp
// Main event loop (in main.cpp)
QApplication app(argc, argv);
return app.exec();  // Enters event loop

// Event loop processes:
// - UI events (mouse, keyboard)
// - Timer events
// - Network events (QNetworkReply)
// - Custom events (QEvent)
// - Signal-slot invocations
```

**Signal-Slot Execution**:
```cpp
// Queued Connection (cross-thread)
connect(obj1, &Class::signal, obj2, &Class::slot, Qt::QueuedConnection);
// Signal emission → Event posted → Event loop → Slot execution

// Direct Connection (same thread) - DEFAULT
connect(obj1, &Class::signal, obj2, &Class::slot, Qt::DirectConnection);
// Signal emission → Immediate slot execution (synchronous)
```

**Network Event Processing**:
```
1. QNetworkAccessManager::post() called
2. HTTP request queued in network subsystem
3. Worker thread sends HTTP request
4. Response data arrives in chunks
5. QNetworkReply emits readyRead() signals
6. All data received
7. QNetworkReply emits finished() signal
8. Connected lambda/slot executes
9. UI updated via signal-slot
10. Event loop continues processing other events
```

### Performance Considerations

**Network Performance**:
- **Connection Reuse**: HTTP/1.1 keep-alive reduces connection overhead
- **Compression**: Could add `Accept-Encoding: gzip` header (not implemented)
- **Timeout Handling**: Default timeout is 30 seconds (configurable)
- **Concurrent Requests**: Multiple requests can be in-flight simultaneously

**UI Responsiveness**:
- **Non-Blocking**: All network I/O is asynchronous
- **Chunked Updates**: Could implement streaming for faster perceived response
- **Progress Indicators**: Could add progress bar for long requests

**Memory Performance**:
- **Block Limits**: `setMaximumBlockCount()` prevents memory leaks
- **String Optimization**: Qt's implicit sharing reduces memory usage
- **Widget Pooling**: Not needed (widgets are lightweight)

### Security Considerations

**API Key Storage**:
- **QSettings**: Stores API key in Windows Registry (encrypted on Windows 10+)
- **No Hardcoding**: API key never in source code
- **User Responsibility**: User must keep API key secure

**Network Security**:
- **HTTPS Only**: All requests use TLS encryption
- **Certificate Validation**: Qt validates SSL certificates automatically
- **No Certificate Pinning**: Relies on system certificate store

**Input Validation**:
- **Empty Checks**: Validates API key and question before sending
- **String Sanitization**: No SQL injection risk (no database)
- **XSS Prevention**: Text displayed as plain text (not HTML)

**Error Information**:
- **User-Friendly Messages**: Errors don't expose internal details
- **No Stack Traces**: Error messages are sanitized for users

---

## Design Patterns Used

### 1. Signal-Slot Pattern (Qt's Event-Driven Communication)

**Pattern Description**:
- **Decoupled Communication**: Objects communicate without knowing each other
- **Type-Safe**: Compile-time type checking (new-style syntax)
- **Thread-Safe**: Signals can be emitted from any thread
- **Flexible**: One signal can connect to multiple slots

**Implementation**:
```cpp
// Signal declaration (in header)
signals:
    void answerReady(const QString& answer);
    void requestFailed(const QString& errorMessage);

// Signal emission (in implementation)
emit answerReady(answer);  // Synchronous emission, async slot execution

// Slot connection (in UI)
connect(assistant, &AIAssistant::answerReady, 
        this, &AIAssistantDock::onAnswerReady);
```

**Connection Types**:
- **Direct Connection** (default): Slot executes immediately in emitter's thread
- **Queued Connection**: Slot executes later in receiver's thread (cross-thread)
- **Auto Connection**: Qt chooses based on thread affinity

**Benefits**:
- **Loose Coupling**: AIAssistant doesn't know about UI
- **Multiple Listeners**: Multiple UI components can listen to same signal
- **Runtime Flexibility**: Connections can be made/removed at runtime

### 2. Observer Pattern

**Pattern Description**:
- **Subject**: AIAssistant (observable)
- **Observers**: AIAssistantDock, AIAssistantDialog (observers)
- **Notification**: Via signals (Qt's implementation of Observer)

**Implementation**:
```cpp
// Subject (AIAssistant) notifies observers
emit answerReady(answer);  // Notifies all connected observers

// Observers (AIAssistantDock) register interest
connect(assistant, &AIAssistant::answerReady, 
        this, &AIAssistantDock::onAnswerReady);
```

**Benefits**:
- **One-to-Many**: One subject can notify multiple observers
- **Dynamic**: Observers can be added/removed at runtime
- **Decoupled**: Subject doesn't need to know observer types

### 3. Separation of Concerns

**Architectural Layers**:

```
┌─────────────────────────────────────┐
│   Presentation Layer (UI)           │
│   - AIAssistantDock                 │
│   - AIAssistantDialog               │
│   - User interaction                │
└─────────────────────────────────────┘
              │ (signals/slots)
              ▼
┌─────────────────────────────────────┐
│   Business Logic Layer              │
│   - AIAssistant                     │
│   - API communication               │
│   - Data processing                 │
└─────────────────────────────────────┘
              │ (HTTP requests)
              ▼
┌─────────────────────────────────────┐
│   Data/Configuration Layer          │
│   - Settings                        │
│   - QSettings persistence           │
└─────────────────────────────────────┘
```

**Benefits**:
- **Maintainability**: Changes to UI don't affect business logic
- **Testability**: Business logic can be tested independently
- **Reusability**: AIAssistant can be used by different UI components

### 4. Factory Pattern (Implicit)

**Pattern Description**:
- **Lazy Initialization**: Dock created on first use
- **Singleton-like**: One dock instance per MainWindow

**Implementation**:
```cpp
AIAssistantDock* MainWindow::ensureAiDock()
{
    if (aiDock) return aiDock;  // Return existing
    // Create new instance
    aiDock = new AIAssistantDock(aiAssistant, this);
    return aiDock;
}
```

### 5. Strategy Pattern (Model Selection)

**Pattern Description**:
- **Algorithm Selection**: Different AI models can be used
- **Runtime Selection**: Model chosen via settings

**Implementation**:
```cpp
QString model = Settings::aiModel();  // Get strategy
assistant->askDeepSeek(apiKey, question, selectedText, model);
```

### 6. RAII (Resource Acquisition Is Initialization)

**Pattern Description**:
- **Automatic Resource Management**: Resources acquired in constructor, released in destructor
- **Exception Safety**: Resources cleaned up even if exceptions occur

**Implementation**:
```cpp
// QNetworkAccessManager automatically manages connections
AIAssistant::AIAssistant(QObject* parent) : QObject(parent)
{
    // network member variable initialized automatically
    // No manual resource acquisition needed
}

// Widgets automatically deleted when parent is deleted
AIAssistantDock::AIAssistantDock(...)
{
    QWidget* root = new QWidget(this);  // Parented
    // Automatically deleted when dock is deleted
}
```

## Qt-Specific Technical Details

### Meta-Object System (MOC)

**MOC Processing**:
```cpp
// Header file (AIAssistant.h)
class AIAssistant : public QObject
{
    Q_OBJECT  // MOC processes this class
    
signals:
    void answerReady(const QString&);
};
```

**MOC Generated Code** (simplified):
```cpp
// moc_AIAssistant.cpp (generated)
const QMetaObject* AIAssistant::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void AIAssistant::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AIAssistant*>(_o);
        switch (_id) {
        case 0: _t->answerReady((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        }
    }
}
```

**MOC Benefits**:
- **Runtime Introspection**: Can query object properties/methods at runtime
- **Signal-Slot**: Enables type-safe signal-slot connections
- **Property System**: Enables Q_PROPERTY for bindings
- **qobject_cast**: Type-safe downcasting

### Qt's String System

**QString vs std::string**:
```cpp
// QString (Qt)
QString str = "Hello";
str += " World";  // Efficient (implicit sharing)

// std::string (STL)
std::string str = "Hello";
str += " World";  // May copy entire string
```

**Implicit Sharing (Copy-on-Write)**:
```cpp
QString str1 = "Hello";
QString str2 = str1;  // No copy! Shares data
str2 += " World";     // Now copies (write triggers copy)
```

**Unicode Support**:
- **Internal**: UTF-16 (2 bytes per character)
- **Conversion**: `toUtf8()` converts to UTF-8 for network/API
- **Locale**: Handles locale-specific string operations

### Qt's Container Classes

**QJsonObject vs std::map**:
```cpp
// QJsonObject (Qt)
QJsonObject obj;
obj["key"] = "value";  // Type-safe, JSON-specific

// std::map (STL)
std::map<std::string, std::string> map;
map["key"] = "value";  // Generic, not JSON-aware
```

**Benefits of Qt Containers**:
- **JSON Integration**: Direct conversion to/from JSON
- **Qt Integration**: Works seamlessly with Qt APIs
- **Memory**: Optimized for Qt's memory model

### Qt's Event System

**Event Loop**:
```cpp
// Main event loop
QApplication app(argc, argv);
return app.exec();  // Blocks until quit()

// Event processing order:
// 1. Window system events (mouse, keyboard)
// 2. Posted events (QCoreApplication::postEvent)
// 3. QTimer events
// 4. Network events (QNetworkReply)
// 5. Custom events
```

**Custom Events** (not used, but possible):
```cpp
class CustomEvent : public QEvent
{
public:
    static const QEvent::Type EventType;
    CustomEvent() : QEvent(EventType) {}
};

// Post custom event
QCoreApplication::postEvent(object, new CustomEvent());
```

### Qt's Property System

**Q_PROPERTY** (not used in this code, but available):
```cpp
class AIAssistant : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    
public:
    QString model() const { return m_model; }
    void setModel(const QString& model) { 
        m_model = model; 
        emit modelChanged(); 
    }
    
signals:
    void modelChanged();
    
private:
    QString m_model;
};
```

**Benefits**:
- **QML Integration**: Properties accessible from QML
- **Reflection**: Can query properties at runtime
- **Binding**: Can bind properties together

### Qt's Resource System

**Resource Files** (used for UI, icons):
```cpp
// resources.qrc
<RCC>
    <qresource>
        <file>icons/icon.png</file>
    </qresource>
</RCC>

// Usage
QPixmap icon(":/icons/icon.png");  // :/ prefix indicates resource
```

**Benefits**:
- **Embedded Resources**: Resources compiled into executable
- **No External Files**: Icons/images don't need separate files
- **Version Control**: Resources in source control

---

## Configuration

### Required Setup
1. User must obtain an OpenRouter API key
2. API key is entered in Settings → IA → OpenRouter API key
3. Optional: Select preferred AI model (default: "deepseek-chat")

### Supported Models
- Any model available through OpenRouter
- Default: DeepSeek Chat (cost-effective, good performance)
- Users can switch to other models (GPT-4, Claude, etc.) via settings

---

## Advantages of This Implementation

1. **Non-Intrusive**: Dock can be hidden/shown as needed
2. **Context-Aware**: Automatically includes selected text in queries
3. **Asynchronous**: Doesn't block the UI during API calls
4. **Error Handling**: Comprehensive error messages for debugging
5. **Flexible**: Supports multiple AI models through OpenRouter
6. **User-Friendly**: Clear status messages and chat history

---

## Code Quality Features

### Error Handling
- **Defensive Programming**: Checks at every step (API key, question, response)
- **Early Returns**: Fail fast with clear error messages
- **Error Propagation**: Errors communicated via signals (not exceptions)
- **User-Friendly Messages**: Technical errors translated to user language

### Input Validation
```cpp
// API Key validation
const QString trimmedKey = apiKey.trimmed();
if (trimmedKey.isEmpty()) {
    emit requestFailed("Missing API key");
    return;
}

// Question validation
if (question.trimmed().isEmpty()) {
    emit requestFailed("Please enter a question");
    return;
}
```

### Resource Management
- **RAII**: All resources managed via constructors/destructors
- **Parent-Child**: Widgets automatically deleted with parent
- **deleteLater()**: Network replies safely deleted in event loop
- **No Raw Pointers**: All pointers are either parented or managed

### User Feedback
- **Status Messages**: Real-time status updates ("Thinking...", "Done")
- **Visual Indicators**: Button/input disabling during requests
- **Error Display**: Errors shown in UI, not just console
- **Chat History**: Persistent conversation history

### Modularity
- **Single Responsibility**: Each class has one clear purpose
- **Low Coupling**: Classes communicate via signals/slots
- **High Cohesion**: Related functionality grouped together
- **Interface Segregation**: Clean, minimal public interfaces

## Performance Analysis

### Time Complexity
- **Network Request**: O(1) - Single HTTP request
- **JSON Parsing**: O(n) - Linear in response size
- **Text Processing**: O(n) - Linear in text length
- **UI Updates**: O(1) - Constant time widget updates

### Space Complexity
- **Request Payload**: O(n) - Linear in question + selection size
- **Response Storage**: O(m) - Linear in AI response size
- **Chat History**: O(k) - Bounded by `setMaximumBlockCount()`
- **Widget Tree**: O(1) - Constant number of widgets

### Bottlenecks
1. **Network Latency**: Primary bottleneck (100ms - 5s typical)
2. **JSON Parsing**: Minor (1-10ms for typical responses)
3. **UI Updates**: Negligible (<1ms)

### Optimization Opportunities
- **Streaming**: Could implement streaming responses for faster perceived performance
- **Caching**: Could cache common questions/responses
- **Compression**: Could enable gzip compression for requests
- **Connection Pooling**: Already handled by Qt (HTTP/1.1 keep-alive)

## Debugging and Troubleshooting

### Debug Techniques

**Qt Debugging Tools**:
```cpp
// qDebug() for logging
qDebug() << "API Key length:" << apiKey.length();
qDebug() << "Question:" << question;
qDebug() << "Response status:" << statusCode;

// Q_ASSERT for invariants
Q_ASSERT(assistant != nullptr);
Q_ASSERT(!apiKey.isEmpty());
```

**Network Debugging**:
```cpp
// Enable network debugging
QLoggingCategory::setFilterRules("qt.network.*=true");

// Inspect request
qDebug() << "Request URL:" << req.url();
qDebug() << "Request headers:" << req.rawHeaderList();
qDebug() << "Request payload:" << jsonData;
```

**Signal-Slot Debugging**:
```cpp
// Check if signal is connected
if (receivers(SIGNAL(answerReady(QString))) > 0) {
    qDebug() << "Signal is connected";
}

// Use QSignalSpy for testing
QSignalSpy spy(assistant, &AIAssistant::answerReady);
// ... trigger signal ...
QCOMPARE(spy.count(), 1);
```

### Common Issues

**Issue 1: Signal Not Received**
- **Cause**: Signal not connected, or receiver deleted
- **Debug**: Check `receivers()` count, verify connection
- **Fix**: Ensure connection made before signal emission

**Issue 2: Network Timeout**
- **Cause**: Slow network or API server issues
- **Debug**: Check `QNetworkReply::error()` for timeout
- **Fix**: Increase timeout or add retry logic

**Issue 3: JSON Parse Error**
- **Cause**: Malformed JSON response
- **Debug**: Log raw response, check `QJsonParseError`
- **Fix**: Handle parse errors gracefully, show raw response

**Issue 4: Memory Leak**
- **Cause**: Forgetting `deleteLater()` on network reply
- **Debug**: Use Qt Creator's memory profiler
- **Fix**: Always call `deleteLater()` in all code paths

## Build System Integration

### CMake Configuration

**CMakeLists.txt**:
```cmake
cmake_minimum_required(VERSION 3.16)
project(UITPad VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt configuration
set(CMAKE_AUTOUIC ON)   # UI files
set(CMAKE_AUTOMOC ON)   # MOC processing
set(CMAKE_AUTORCC ON)   # Resource files

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# Source files
set(PROJECT_SOURCES
    "Source Files/AIAssistant.cpp"
    "Header Files/AIAssistant.h"
    # ... other files
)

# Create executable
qt_add_executable(UITPad ${PROJECT_SOURCES})

# Link libraries
target_link_libraries(UITPad PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Network
)
```

**MOC Processing**:
- **Automatic**: CMake automatically runs MOC on files with `Q_OBJECT`
- **Generated Files**: `moc_AIAssistant.cpp` generated automatically
- **Dependencies**: CMake tracks MOC dependencies

**Build Process**:
```
1. CMake configuration
2. MOC processes Q_OBJECT classes
3. UIC processes .ui files
4. RCC processes .qrc files
5. C++ compilation
6. Linking
```

### Compilation Details

**Preprocessor Steps**:
1. **MOC**: Processes `Q_OBJECT` classes → generates `moc_*.cpp`
2. **UIC**: Processes `.ui` files → generates `ui_*.h`
3. **RCC**: Processes `.qrc` files → generates `qrc_*.cpp`
4. **C++ Compiler**: Compiles all `.cpp` files

**Generated Files** (in build directory):
```
build/
├── UITPad_autogen/
│   ├── mocs_compilation.cpp
│   └── include/
│       └── moc_AIAssistant.cpp
└── ...
```

**Dependencies**:
- **Qt6/Qt5**: Widgets, Network modules
- **C++17**: Required for lambda captures, structured bindings
- **CMake 3.16+**: Required for Qt6 support

## Testing Considerations

### Unit Testing (Not Implemented, But Possible)

**Qt Test Framework**:
```cpp
// TestAIAssistant.cpp
#include <QtTest>
#include "AIAssistant.h"

class TestAIAssistant : public QObject
{
    Q_OBJECT
    
private slots:
    void testBuildUserContent();
    void testEmptyApiKey();
    void testEmptyQuestion();
};

void TestAIAssistant::testBuildUserContent()
{
    QString result = AIAssistant::buildUserContent("Question", "Selected");
    QVERIFY(result.contains("Question"));
    QVERIFY(result.contains("Selected"));
}
```

### Integration Testing
- **Mock Network**: Use `QNetworkAccessManager` with mock responses
- **UI Testing**: Use Qt Test's GUI testing features
- **End-to-End**: Test full flow from UI to API response

### Manual Testing Checklist
- [ ] API key validation
- [ ] Empty question handling
- [ ] Network error handling
- [ ] JSON parse error handling
- [ ] UI state updates (busy/idle)
- [ ] Chat history persistence
- [ ] Text selection sync
- [ ] Dock show/hide
- [ ] Settings persistence

---

## Future Enhancement Possibilities

1. **Streaming Responses**: Real-time token streaming for faster perceived response
2. **Multiple Conversations**: Support for multiple chat sessions
3. **Code Actions**: Direct code insertion/editing capabilities
4. **Local Models**: Support for locally-running AI models
5. **Context Memory**: Remember conversation context across sessions

---

## Advanced Technical Topics

### Thread Safety

**Current Implementation**:
- **Single-Threaded**: All operations in main (UI) thread
- **Thread-Safe Components**: `QNetworkAccessManager` is thread-safe
- **Signal-Slot Thread Safety**: Signals can be emitted from any thread

**Potential Multi-Threading**:
```cpp
// Could move network operations to worker thread
class NetworkWorker : public QObject
{
    Q_OBJECT
public slots:
    void processRequest(QNetworkRequest req, QByteArray data);
signals:
    void responseReady(QByteArray data);
};

// In worker thread
QThread* workerThread = new QThread;
NetworkWorker* worker = new NetworkWorker;
worker->moveToThread(workerThread);
workerThread->start();
```

**Benefits of Current Approach**:
- **Simplicity**: No thread synchronization needed
- **Qt Integration**: UI updates must be in main thread anyway
- **Performance**: Network I/O is already asynchronous

### Memory Management Deep Dive

**Qt's Ownership Model**:
```cpp
// Parent-child ownership
QWidget* parent = new QWidget();
QPushButton* child = new QPushButton(parent);
// When parent deleted, child automatically deleted
// No memory leak, no manual delete needed
```

**Smart Pointers** (Not Used, But Available):
```cpp
// Could use std::unique_ptr or QScopedPointer
std::unique_ptr<QNetworkReply> reply(network.post(req, data));
// Automatically deleted when out of scope
```

**Why Raw Pointers Are OK Here**:
- **Qt's Model**: Parent-child ownership is Qt's standard pattern
- **Lifetime**: Widgets live as long as parent (entire application)
- **No Leaks**: Qt's destructor chain ensures cleanup

### Lambda Captures

**Current Lambda Usage**:
```cpp
connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    // [this, reply] - capture by value
    // this - needed to access member functions
    // reply - needed to read response
});
```

**Capture Options**:
- `[this]` - Capture `this` pointer
- `[reply]` - Capture `reply` by value (copy pointer)
- `[&reply]` - Capture by reference (dangerous if object deleted)
- `[=]` - Capture all by value (expensive)
- `[&]` - Capture all by reference (dangerous)

**Best Practice**: Capture only what's needed, by value

### JSON Performance

**QJsonDocument Performance**:
- **Parsing**: O(n) where n is JSON size
- **Memory**: Creates full object tree in memory
- **Optimization**: `QJsonDocument::Compact` reduces size

**Alternative Approaches**:
```cpp
// Streaming JSON parser (not used, but available)
QJsonStreamReader reader(data);
while (reader.readNext()) {
    if (reader.key() == "content") {
        QString content = reader.value().toString();
    }
}
```

**Current Approach Benefits**:
- **Simplicity**: Full parse, then access
- **Validation**: Complete JSON structure validated
- **Performance**: Adequate for typical API responses (<1MB)

### Network Protocol Details

**HTTP/1.1 Features Used**:
- **Keep-Alive**: Connection reuse (automatic in Qt)
- **Chunked Transfer**: Handled automatically
- **Compression**: Not used (could add `Accept-Encoding: gzip`)

**TLS/SSL Details**:
- **Version**: TLS 1.2+ (Qt default)
- **Cipher Suites**: System default
- **Certificate Validation**: System certificate store
- **No Pinning**: Relies on system trust

**Request Headers**:
```
POST /api/v1/chat/completions HTTP/1.1
Host: openrouter.ai
Content-Type: application/json
Accept: application/json
Authorization: Bearer sk-...
HTTP-Referer: https://uitpad.local
X-Title: UITPad
Content-Length: 1234
```

### Error Recovery Strategies

**Current Strategy**: Fail fast, show error
**Alternative Strategies**:

1. **Retry Logic**:
```cpp
void retryRequest(int maxRetries = 3) {
    for (int i = 0; i < maxRetries; ++i) {
        if (sendRequest()) return;
        QThread::msleep(1000 * (i + 1));  // Exponential backoff
    }
}
```

2. **Fallback Models**:
```cpp
QStringList models = {"deepseek-chat", "gpt-3.5-turbo", "claude-3-haiku"};
for (const QString& model : models) {
    if (tryRequest(model)) break;
}
```

3. **Caching**:
```cpp
QHash<QString, QString> responseCache;
if (responseCache.contains(cacheKey)) {
    return responseCache[cacheKey];
}
```

### API Rate Limiting

**Current Handling**: Shows error on 429 status
**Advanced Handling**:
```cpp
if (statusCode == 429) {
    // Parse Retry-After header
    QString retryAfter = reply->rawHeader("Retry-After");
    int seconds = retryAfter.toInt();
    
    // Queue request for later
    QTimer::singleShot(seconds * 1000, this, [this]() {
        retryRequest();
    });
}
```

### Security Hardening

**Current Security**:
- ✅ HTTPS only
- ✅ API key in settings (not hardcoded)
- ✅ Input validation
- ✅ No XSS (plain text display)

**Additional Hardening** (Not Implemented):
- **API Key Encryption**: Encrypt API key in settings
- **Request Signing**: Sign requests with HMAC
- **Certificate Pinning**: Pin OpenRouter certificate
- **Input Sanitization**: More aggressive input validation
- **Rate Limiting**: Client-side rate limiting

## Summary

The AI Assistant feature transforms UITPad from a simple text editor into an intelligent coding assistant. It seamlessly integrates AI capabilities into the editing workflow, allowing users to get help without leaving the application.

### Technical Highlights

**Architecture**:
- Clean separation of concerns (UI, business logic, data)
- Signal-slot pattern for decoupled communication
- Qt's parent-child ownership for automatic memory management

**Implementation**:
- Asynchronous network I/O with `QNetworkAccessManager`
- Type-safe JSON parsing with `QJsonDocument`
- Comprehensive error handling at all levels
- Efficient memory usage with bounded containers

**Qt Integration**:
- MOC-based signal-slot system
- Event-driven architecture
- Automatic resource management
- Cross-platform compatibility

**Performance**:
- Non-blocking UI (asynchronous I/O)
- Efficient string handling (implicit sharing)
- Bounded memory usage (maximum block counts)
- Connection reuse (HTTP keep-alive)

The implementation follows Qt best practices, uses modern C++17 features, implements robust error handling, and provides a maintainable, extensible codebase suitable for production use.

