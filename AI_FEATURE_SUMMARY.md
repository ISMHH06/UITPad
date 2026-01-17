# AI Assistant Feature - Quick Summary for Presentation

## What It Does (30 seconds)
The AI Assistant allows users to:
- Select text in their document and ask questions about it
- Get AI-powered explanations, debugging help, or code suggestions
- Interact through a dockable panel that stays open while editing

---

## How It Works (1 minute)

### Architecture (3 Components)
1. **AIAssistant** - Backend class that communicates with the AI API
2. **AIAssistantDock** - UI panel that shows chat history and input
3. **AIAssistantDialog** - Alternative popup dialog interface

### Technical Flow
```
User selects text → Types question → AIAssistant sends HTTP request 
→ OpenRouter API processes → Response parsed → Displayed in UI
```

### API Integration
- Uses **OpenRouter** service (OpenAI-compatible API)
- Supports multiple AI models (DeepSeek, GPT-4, Claude, etc.)
- Asynchronous requests (non-blocking UI)
- Comprehensive error handling

---

## Key Technical Features

### 1. Context-Aware
- Automatically includes selected text in the AI prompt
- User question + selected code = better AI responses

### 2. Asynchronous Communication
- Uses Qt's `QNetworkAccessManager`
- UI remains responsive during API calls
- Signal-slot pattern for response handling

### 3. Error Handling
- Network errors
- API authentication errors
- Rate limiting
- User-friendly error messages

### 4. User Experience
- Dockable panel (can move around)
- Chat history maintained
- Status updates ("Thinking...", "Done")
- Keyboard shortcuts support

---

## Code Structure

```
AIAssistant.cpp
├── askDeepSeek() - Main API call method
├── buildUserContent() - Formats prompt with context
└── Network reply handler - Parses JSON response

AIAssistantDock.cpp
├── UI setup - Selection view, chat view, input
├── onSend() - Handles user input
├── onAnswerReady() - Updates UI with response
└── Error handling - Displays error messages
```

---

## Design Patterns Used
- **Signal-Slot**: Qt's event-driven communication
- **Observer Pattern**: UI observes AIAssistant state
- **Separation of Concerns**: Logic vs. UI separation

---

## Configuration
- API key stored in Settings
- Model selection (default: DeepSeek)
- Accessible via Settings → IA menu

---

## Why This Implementation is Good
✅ Non-blocking (async)  
✅ Error-resistant  
✅ User-friendly  
✅ Modular and maintainable  
✅ Follows Qt best practices  

---

## Demo Points to Show
1. Select code → Ask question → Get explanation
2. Show chat history accumulating
3. Show error handling (if API key missing)
4. Show dock can be moved/hidden

---

## One-Sentence Summary
"An integrated AI assistant that uses OpenRouter API to provide context-aware coding help directly within the text editor, with a dockable chat interface that maintains conversation history."

