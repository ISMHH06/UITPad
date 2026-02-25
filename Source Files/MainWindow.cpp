#include "MainWindow.h"
#include "TextEditor.h"
#include "Document.h"
#include "SpellChecker.h"
#include "Settings.h"
#include "HybridHighlighter.h"
#include "AIAssistant.h"
#include "AISettingsDialog.h"
#include "IAChatWidget.h"
#include "ThemeManager.h"
#include "CodeEditorWidget.h"

#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequence>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QTextCursor>
#include <QInputDialog>
#include <QFileInfo>
#include <QDir>
#include <QStatusBar>
#include <QToolButton>
#include <QToolBar>
#include <QEvent>
#include <QApplication>
#include <QClipboard>
#include <QTreeView>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QLabel>
#include <QSplitter>
#include <QPushButton>
#include <QRegularExpression>
#include <QListWidget>
#include <QPixmap>
#include <QPainter>
#include <QStyle>
#include <QProxyStyle>
#include <QTabBar>
#include <QPen>

// Custom style to provide a close button icon for tabs
class TabCloseButtonStyle : public QProxyStyle {
public:
    using QProxyStyle::QProxyStyle;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr,
         const QWidget *widget = nullptr) const override {
      if (metric == PM_TabCloseIndicatorWidth || metric == PM_TabCloseIndicatorHeight)
   return 16;
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

// ═══════════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    spellChecker = new SpellChecker(":/dictionary.txt");
 textEditor = new TextEditor(this);

    // Initialize AI Assistant
    aiAssistant = new AIAssistant(this);
    if (!aiAssistant) {
        qCritical() << "ERROR: Cannot create AIAssistant";
        return;
    }

    // Initialize AI Chat Widget
    chatWidget = new IAChatWidget(aiAssistant, this);
    if (!chatWidget) {
      qCritical() << "ERROR: Cannot create IAChatWidget";
        return;
    }

  // Initialize compiler manager
    compilerManager = new CompilerManager(this);

    // Initialize output window
    outputWindow = new OutputWindow(this);
    addDockWidget(Qt::BottomDockWidgetArea, outputWindow);
  outputWindow->hide();

    // Connect compiler signals
    connect(compilerManager, &CompilerManager::compilationStarted,
        this, [this](const QString&) {
    outputWindow->clearCompileOutput();
      outputWindow->show();
    outputWindow->showCompileTab();
            if (statusBuildState) statusBuildState->setText("  Building...  ");
        });

    connect(compilerManager, &CompilerManager::compilationOutput,
        outputWindow, &OutputWindow::appendCompileOutput);

 connect(compilerManager, &CompilerManager::compilationError,
   outputWindow, &OutputWindow::appendCompileError);

    connect(compilerManager, &CompilerManager::compilationFinished,
      this, [this](bool success, int) {
            if (success) {
   statusBar()->showMessage("Compilation successful", 3000);
     if (statusBuildState) statusBuildState->setText("  Ready  ");
   }
            else {
      statusBar()->showMessage("Compilation failed", 3000);
        if (statusBuildState) statusBuildState->setText("  Error  ");
            }
     });

    connect(compilerManager, &CompilerManager::executionStarted,
        this, [this](const QString&) {
     outputWindow->clearRunOutput();
            outputWindow->show();
      outputWindow->showRunTab();
        if (statusBuildState) statusBuildState->setText("  Running...  ");
     });

connect(compilerManager, &CompilerManager::executionOutput,
        outputWindow, &OutputWindow::appendRunOutput);

    connect(compilerManager, &CompilerManager::executionError,
        outputWindow, &OutputWindow::appendRunError);

    connect(compilerManager, &CompilerManager::executionFinished,
        this, [this](int exitCode) {
      statusBar()->showMessage(
    QString("Program finished with exit code %1").arg(exitCode), 3000);
            if (statusBuildState) statusBuildState->setText("  Ready  ");
   });

    connect(outputWindow, &OutputWindow::stopCompilationRequested,
        compilerManager, &CompilerManager::stopCompilation);

 connect(outputWindow, &OutputWindow::stopExecutionRequested,
        compilerManager, &CompilerManager::stopExecution);

    // ── Tab Widget (central) ────────────────────────────────────
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
  tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);  // Cleaner tab appearance
    tabWidget->setElideMode(Qt::ElideRight);
    setCentralWidget(tabWidget);

    // Create close button icon for tabs
    createTabCloseIcon();

    connect(tabWidget, &QTabWidget::currentChanged,
        this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested,
     this, &MainWindow::onTabCloseRequested);

    connect(textEditor, &TextEditor::documentOpened,
        this, &MainWindow::onDocumentOpened);
    connect(textEditor, &TextEditor::documentClosed,
        this, &MainWindow::onDocumentClosed);
    connect(textEditor, &TextEditor::documentSaved,
        this, &MainWindow::onDocumentSaved);
    connect(textEditor, &TextEditor::currentDocumentChanged,
        this, &MainWindow::onCurrentDocumentChanged);

    // ── Setup UI Components ─────────────────────────────────────
    setupMenus();
    setupToolBar();
    setupProjectExplorer();
    setupStatusBar();

    setWindowTitle("UITPad");

    // ── Initialize theme ────────────────────────────────────────
    Settings s(this);
    currentTheme = s.getSelectedTheme();
    if (currentTheme == Settings::System) {
        bool isDark = Settings::isSystemDarkMode();
        currentTheme = isDark ? Settings::Dark : Settings::Light;
    }
    applySystemTheme();

    resize(1100, 700);

    // ── Add AI Chat Widget as dock ──────────────────────────────
    addDockWidget(Qt::RightDockWidgetArea, chatWidget);
    chatWidget->hide();

    // Connect AI signals
    if (aiAssistant) {
        connect(aiAssistant, &AIAssistant::errorOccurred, this, &MainWindow::onAIError);
      connect(chatWidget, &IAChatWidget::codeInsertRequested, this, &MainWindow::onCodeInsertRequested);
        connect(chatWidget, &IAChatWidget::codeCopyRequested, this, &MainWindow::onCodeCopyRequested);
        connect(aiAssistant, &AIAssistant::statusMessage, [this](const QString& msg) {
     statusBar()->showMessage(msg, 3000);
        });
    }

    qDebug() << "MainWindow created successfully";
}

MainWindow::~MainWindow() {
  delete spellChecker;
}

// ═══════════════════════════════════════════════════════════════════
//  Tab Close Icon
// ═══════════════════════════════════════════════════════════════════
void MainWindow::createTabCloseIcon() {
    // Create a small × icon for the tab close button
    auto makeClosePixmap = [](QColor color, int size = 16) -> QPixmap {
        QPixmap pix(size, size);
        pix.fill(Qt::transparent);
QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        QPen pen(color, 1.5);
        p.setPen(pen);
      int m = 4;  // margin
        p.drawLine(m, m, size - m, size - m);
        p.drawLine(size - m, m, m, size - m);
        p.end();
        return pix;
};

    QPixmap closePix = makeClosePixmap(QColor("#828997"));
    QPixmap closeHoverPix = makeClosePixmap(QColor("#D7DAE0"));

    // Save to temp files and set via stylesheet
    closePix.save(QDir::tempPath() + "/uitpad_close.png");
    closeHoverPix.save(QDir::tempPath() + "/uitpad_close_hover.png");

    QString closeIconPath = QDir::tempPath() + "/uitpad_close.png";
    QString closeHoverPath = QDir::tempPath() + "/uitpad_close_hover.png";

    // Convert to proper URL format for stylesheet
    closeIconPath.replace("\\", "/");
    closeHoverPath.replace("\\", "/");

    tabWidget->tabBar()->setStyleSheet(
        QString(
     "QTabBar::close-button {"
       "  image: url(%1);"
            "  subcontrol-position: right;"
            "  padding: 2px;"
          "  margin: 2px;"
            "}"
            "QTabBar::close-button:hover {"
     "  image: url(%2);"
        "  background-color: rgba(255, 255, 255, 30);"
            "  border-radius: 3px;"
          "}"
        ).arg(closeIconPath, closeHoverPath)
    );
}

// ═══════════════════════════════════════════════════════════════════
//  Setup: Menus
// ═══════════════════════════════════════════════════════════════════
void MainWindow::setupMenus() {
    // Menu Fichier
 QMenu* fileMenu = menuBar()->addMenu("  File  ");
    fileMenu->addAction("New File", QKeySequence::New, this, &MainWindow::onFileNew);
    fileMenu->addAction("Open File...", QKeySequence::Open, this, &MainWindow::onFileOpen);
    fileMenu->addAction("Open Folder...", QKeySequence("Ctrl+K"), this, &MainWindow::onOpenFolder);
    fileMenu->addAction("Rename...", this, &MainWindow::onFileRename);
    fileMenu->addSeparator();
    fileMenu->addAction("Save", QKeySequence::Save, this, &MainWindow::onFileSave);
    fileMenu->addAction("Save As...", QKeySequence::SaveAs, this, &MainWindow::onFileSaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Close File", QKeySequence::Close, this, &MainWindow::onFileClose);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", QKeySequence::Quit, this, &QWidget::close);

    // Menu Edit
    QMenu* editMenu = menuBar()->addMenu("  Edit  ");
    editMenu->addAction("Settings...", this, &MainWindow::onOpenSettings);

    // Menu View
    QMenu* viewMenu = menuBar()->addMenu("  View  ");

    QAction* explorerAction = viewMenu->addAction("Project Explorer");
    explorerAction->setShortcut(QKeySequence("Ctrl+B"));
    explorerAction->setCheckable(true);
    explorerAction->setChecked(true);
    connect(explorerAction, &QAction::triggered, this, &MainWindow::toggleProjectExplorer);

    viewMenu->addSeparator();

    QAction* syntaxAction = viewMenu->addAction("Syntax Highlighting",
        this, &MainWindow::onToggleSyntaxHighlighting);
    syntaxAction->setCheckable(true);
 syntaxAction->setChecked(isSyntaxHighlightingEnabled);

    QAction* spellAction = viewMenu->addAction("Spell Check",
        this, &MainWindow::onToggleSpellCheck);
    spellAction->setCheckable(true);
    spellAction->setChecked(isSpellCheckEnabled);

    // Menu Run
    QMenu* runMenu = menuBar()->addMenu("  Run  ");
    runMenu->addAction("Compile", QKeySequence("F7"), this, &MainWindow::onCompile);
    runMenu->addAction("Compile && Run", QKeySequence("Ctrl+F5"), this, &MainWindow::onCompileAndRun);
    runMenu->addAction("Run", QKeySequence("F5"), this, &MainWindow::onRun);
    runMenu->addSeparator();
    runMenu->addAction("Stop Compilation", QKeySequence("Shift+F7"), this, &MainWindow::onStopCompilation);
    runMenu->addAction("Stop Execution", QKeySequence("Shift+F5"), this, &MainWindow::onStopExecution);
    runMenu->addSeparator();
    runMenu->addAction("Compiler Settings...", this, &MainWindow::onCompilerSettings);

    // Menu AI
    QMenu* aiMenu = menuBar()->addMenu("  AI  ");

    QAction* explainAction = aiMenu->addAction("Explain Selection");
    explainAction->setShortcut(QKeySequence("Ctrl+Shift+E"));
    connect(explainAction, &QAction::triggered, this, &MainWindow::onExplainCode);

    QAction* completeAction = aiMenu->addAction("Complete Code");
    completeAction->setShortcut(QKeySequence("Ctrl+Shift+Space"));
    connect(completeAction, &QAction::triggered, this, &MainWindow::onCompleteCode);

    QAction* commentAction = aiMenu->addAction("Generate from Comment");
    commentAction->setShortcut(QKeySequence("Ctrl+Shift+G"));
    connect(commentAction, &QAction::triggered, this, &MainWindow::onGenerateFromComment);

    aiMenu->addSeparator();

    QAction* toggleChatAction = aiMenu->addAction("Toggle AI Chat");
    toggleChatAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    connect(toggleChatAction, &QAction::triggered, this, &MainWindow::toggleChatWidget);

    QAction* aiSettingsAction = aiMenu->addAction("AI Settings...");
    connect(aiSettingsAction, &QAction::triggered, this, &MainWindow::onAISettings);
}

// ═══════════════════════════════════════════════════════════════════
//  Setup: Toolbar
// ═══════════════════════════════════════════════════════════════════
void MainWindow::setupToolBar() {
    mainToolBar = new QToolBar("Main", this);
    mainToolBar->setMovable(false);
    mainToolBar->setIconSize(QSize(16, 16));
mainToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

  // File actions
    QAction* newAct = mainToolBar->addAction("New");
  newAct->setToolTip("New File (Ctrl+N)");
    connect(newAct, &QAction::triggered, this, &MainWindow::onFileNew);

    QAction* openAct = mainToolBar->addAction("Open");
    openAct->setToolTip("Open File (Ctrl+O)");
    connect(openAct, &QAction::triggered, this, &MainWindow::onFileOpen);

    QAction* saveAct = mainToolBar->addAction("Save");
    saveAct->setToolTip("Save (Ctrl+S)");
  connect(saveAct, &QAction::triggered, this, &MainWindow::onFileSave);

mainToolBar->addSeparator();

    // Build actions
 QAction* compileAct = mainToolBar->addAction("Compile");
    compileAct->setToolTip("Compile (F7)");
    connect(compileAct, &QAction::triggered, this, &MainWindow::onCompile);

    QAction* runAct = mainToolBar->addAction("Run");
    runAct->setToolTip("Run (F5)");
    connect(runAct, &QAction::triggered, this, &MainWindow::onRun);

    QAction* compRunAct = mainToolBar->addAction("Build & Run");
    compRunAct->setToolTip("Compile and Run (Ctrl+F5)");
    connect(compRunAct, &QAction::triggered, this, &MainWindow::onCompileAndRun);

    QAction* stopAct = mainToolBar->addAction("Stop");
    stopAct->setToolTip("Stop Execution (Shift+F5)");
    connect(stopAct, &QAction::triggered, this, &MainWindow::onStopExecution);

    mainToolBar->addSeparator();

    // AI actions
    QAction* aiChatAct = mainToolBar->addAction("AI Chat");
    aiChatAct->setToolTip("Toggle AI Chat (Ctrl+Shift+C)");
    connect(aiChatAct, &QAction::triggered, this, &MainWindow::toggleChatWidget);

    addToolBar(Qt::TopToolBarArea, mainToolBar);
}

// ═══════════════════════════════════════════════════════════════════
//  Setup: Project Explorer
// ═══════════════════════════════════════════════════════════════════
void MainWindow::setupProjectExplorer() {
    projectDock = new QDockWidget("EXPLORER", this);
    projectDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
  projectDock->setMinimumWidth(180);

    fileSystemModel = new QFileSystemModel(this);

    QStringList filters;
    filters << "*.cpp" << "*.h" << "*.hpp" << "*.c" << "*.cc"
     << "*.cxx" << "*.hxx" << "*.inl" << "*.txt"
        << "*.md" << "*.json" << "*.xml" << "*.cmake"
        << "CMakeLists.txt" << "*.pro" << "*.qrc" << "*.ui";
    fileSystemModel->setNameFilters(filters);
    fileSystemModel->setNameFilterDisables(false);
    fileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    projectFilterProxy = new ProjectFilterProxy(this);
    projectFilterProxy->setSourceModel(fileSystemModel);
    projectFilterProxy->setRecursiveFilteringEnabled(true);

  // ── Main container ──────────────────────────────────────────
    QWidget* explorerContainer = new QWidget(projectDock);
    QVBoxLayout* explorerLayout = new QVBoxLayout(explorerContainer);
    explorerLayout->setContentsMargins(0, 0, 0, 0);
    explorerLayout->setSpacing(0);

    // ── SECTION 1: OPEN EDITORS ─────────────────────────────────
    openEditorsContainer = new QWidget(explorerContainer);
    openEditorsContainer->setObjectName("openEditorsContainer");
    QVBoxLayout* oeLayout = new QVBoxLayout(openEditorsContainer);
    oeLayout->setContentsMargins(0, 0, 0, 0);
    oeLayout->setSpacing(0);

    // Header (clickable to collapse/expand)
    openEditorsHeader = new QWidget(openEditorsContainer);
    openEditorsHeader->setObjectName("sectionHeader");
  openEditorsHeader->setCursor(Qt::PointingHandCursor);
    openEditorsHeader->setFixedHeight(24);
    openEditorsHeader->installEventFilter(this);
    QHBoxLayout* oeHeaderLayout = new QHBoxLayout(openEditorsHeader);
    oeHeaderLayout->setContentsMargins(8, 0, 8, 0);
    oeHeaderLayout->setSpacing(4);

    openEditorsArrow = new QLabel("▾", openEditorsHeader);
    openEditorsArrow->setFixedWidth(12);
    openEditorsArrow->setStyleSheet("font-size: 10px; border: none; background: transparent;");
    QLabel* oeTitle = new QLabel("OPEN EDITORS", openEditorsHeader);
    oeTitle->setObjectName("sectionTitle");
    oeTitle->setStyleSheet("font-size: 11px; font-weight: 700; letter-spacing: 0.5px; border: none; background: transparent;");

    oeHeaderLayout->addWidget(openEditorsArrow);
    oeHeaderLayout->addWidget(oeTitle);
    oeHeaderLayout->addStretch();

    // Open editors list
    openEditorsList = new QListWidget(openEditorsContainer);
    openEditorsList->setObjectName("openEditorsList");
    openEditorsList->setMaximumHeight(150);
    openEditorsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    openEditorsList->setFrameShape(QFrame::NoFrame);
    openEditorsList->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(openEditorsList, &QListWidget::itemClicked,
      this, &MainWindow::onOpenEditorItemClicked);

    oeLayout->addWidget(openEditorsHeader);
    oeLayout->addWidget(openEditorsList);

    // ── SECTION 2: FOLDER ───────────────────────────────────────
    folderContainer = new QWidget(explorerContainer);
    folderContainer->setObjectName("folderContainer");
    QVBoxLayout* folderLayout = new QVBoxLayout(folderContainer);
    folderLayout->setContentsMargins(0, 0, 0, 0);
    folderLayout->setSpacing(0);

    // Folder header (clickable)
    folderHeader = new QWidget(folderContainer);
    folderHeader->setObjectName("sectionHeader");
    folderHeader->setCursor(Qt::PointingHandCursor);
    folderHeader->setFixedHeight(24);
    folderHeader->installEventFilter(this);
    QHBoxLayout* folderHeaderLayout = new QHBoxLayout(folderHeader);
    folderHeaderLayout->setContentsMargins(8, 0, 8, 0);
 folderHeaderLayout->setSpacing(4);

    folderArrow = new QLabel("▾", folderHeader);
    folderArrow->setFixedWidth(12);
    folderArrow->setStyleSheet("font-size: 10px; border: none; background: transparent;");
    folderTitleLabel = new QLabel("NO FOLDER OPENED", folderHeader);
    folderTitleLabel->setObjectName("sectionTitle");
    folderTitleLabel->setStyleSheet("font-size: 11px; font-weight: 700; letter-spacing: 0.5px; border: none; background: transparent;");

    folderHeaderLayout->addWidget(folderArrow);
    folderHeaderLayout->addWidget(folderTitleLabel);
    folderHeaderLayout->addStretch();

    // Folder tree view
    projectTree = new QTreeView(folderContainer);
    projectTree->setModel(projectFilterProxy);
    projectTree->hideColumn(1);
    projectTree->hideColumn(2);
    projectTree->hideColumn(3);
    projectTree->setHeaderHidden(true);
    projectTree->setAnimated(true);
    projectTree->setIndentation(16);
    projectTree->setExpandsOnDoubleClick(true);
    projectTree->hide();  // Hidden until a folder is opened

    // Placeholder widget shown when no folder is open
    placeholderWidget = new QWidget(folderContainer);
    placeholderWidget->setObjectName("explorerPlaceholder");
    QVBoxLayout* phLayout = new QVBoxLayout(placeholderWidget);
    phLayout->setContentsMargins(16, 20, 16, 16);
    phLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel* phLabel = new QLabel("You have not yet opened a folder.", placeholderWidget);
    phLabel->setAlignment(Qt::AlignCenter);
    phLabel->setWordWrap(true);
    phLabel->setStyleSheet("color: #828997; font-size: 12px; border: none; background: transparent;");

    QPushButton* openFolderBtn = new QPushButton("Open Folder", placeholderWidget);
    openFolderBtn->setObjectName("openFolderBtn");
    openFolderBtn->setCursor(Qt::PointingHandCursor);
    connect(openFolderBtn, &QPushButton::clicked, this, &MainWindow::onOpenFolder);

    phLayout->addWidget(phLabel);
    phLayout->addSpacing(10);
    phLayout->addWidget(openFolderBtn);
 phLayout->addStretch();

    folderLayout->addWidget(folderHeader);
    folderLayout->addWidget(placeholderWidget);
    folderLayout->addWidget(projectTree);

    connect(projectTree, &QTreeView::doubleClicked,
        this, &MainWindow::onProjectFileClicked);

  // ── Assemble layout ─────────────────────────────────────────
    explorerLayout->addWidget(openEditorsContainer);
    explorerLayout->addWidget(folderContainer);
    explorerLayout->addStretch();

    projectDock->setWidget(explorerContainer);
    addDockWidget(Qt::LeftDockWidgetArea, projectDock);

    explorerHasFolder = false;
}

// ═══════════════════════════════════════════════════════════════════
//  Setup: Status Bar
// ═══════════════════════════════════════════════════════════════════
void MainWindow::setupStatusBar() {
    // Build state (left side)
    statusBuildState = new QLabel("  Ready  ", this);
    statusBuildState->setMinimumWidth(80);
    statusBar()->addWidget(statusBuildState);

    // Spacer
    QLabel* spacer = new QLabel(this);
    statusBar()->addWidget(spacer, 1);

    // File type (right side)
 statusFileType = new QLabel("  Plain Text  ", this);
    statusBar()->addPermanentWidget(statusFileType);

    // Encoding
    statusEncoding = new QLabel("  UTF-8  ", this);
    statusBar()->addPermanentWidget(statusEncoding);

    // Line : Column
    statusLineCol = new QLabel("  Ln 1, Col 1  ", this);
    statusLineCol->setMinimumWidth(100);
    statusBar()->addPermanentWidget(statusLineCol);
}

// ═══════════════════════════════════════════════════════════════════
//  File Actions
// ═══════════════════════════════════════════════════════════════════
void MainWindow::onFileNew() {
    textEditor->createNewDocument();
}

void MainWindow::onFileOpen() {
    QString filename = QFileDialog::getOpenFileName(this, "Open File");
    if (filename.isEmpty()) return;

    Document* doc = textEditor->openDocument(filename);
    if (!doc) {
    QMessageBox::warning(this, "Error", "Cannot open file.");
    }
}

void MainWindow::onFileRename() {
Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

 if (!doc->hasFilePath()) {
        onFileSaveAs();
        return;
    }

    QString oldPath = doc->getFilePath();
    QFileInfo fileInfo(oldPath);
    QString oldName = fileInfo.fileName();

    bool ok;
QString newName = QInputDialog::getText(this, "Rename File",
      "New name:", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        QString newPath = fileInfo.absolutePath() + "/" + newName;
        QFile file(oldPath);

 if (file.rename(newPath)) {
    doc->setFilePath(newPath);
    updateTabTitle(doc);
            updateWindowTitle();
            QMessageBox::information(this, "Success", "File renamed successfully.");
    }
        else {
   QMessageBox::warning(this, "Error", "Cannot rename file.");
        }
    }
}

void MainWindow::onFileSave() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

    if (!doc->hasFilePath()) {
        onFileSaveAs();
        return;
    }
    if (!textEditor->saveDocument(doc)) {
        QMessageBox::warning(this, "Error", "Cannot save file.");
    }
}

void MainWindow::onFileSaveAs() {
 Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

    QString filename = QFileDialog::getSaveFileName(this, "Save As...");
    if (filename.isEmpty()) return;

    doc->setFilePath(filename);
    if (!textEditor->saveDocument(doc)) {
        QMessageBox::warning(this, "Error", "Cannot save file.");
    }
}

void MainWindow::onFileClose() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) return;

    if (doc->getIsModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
  this, "Unsaved Changes",
    "Save changes before closing?",
 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (reply == QMessageBox::Save) onFileSave();
      else if (reply == QMessageBox::Cancel) return;
    }
    textEditor->closeDocument(doc);
}

// ═══════════════════════════════════════════════════════════════════
//  Document Events
// ═══════════════════════════════════════════════════════════════════
void MainWindow::onDocumentOpened(Document* doc) {
    if (doc) {
        // Use CodeEditorWidget instead of plain QPlainTextEdit
     CodeEditorWidget* textEdit = new CodeEditorWidget(this);
        textEdit->setPlainText(doc->getContent());

     textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(textEdit, &QPlainTextEdit::customContextMenuRequested,
  this, &MainWindow::showContextMenu);

   // Set editor font
        Settings s(this);
        QFont editorFont = s.getEditorFont();
    editorFont.setFamily("Cascadia Code");
        if (editorFont.pointSize() < 11)
            editorFont.setPointSize(11);
        textEdit->setFont(editorFont);

        int index = tabWidget->addTab(textEdit, doc->getFileName());
    tabWidget->setCurrentIndex(index);
     textEdit->setProperty("document", QVariant::fromValue(doc));

    // Apply hybrid highlighter
  applyHybridHighlighter(textEdit);

        connect(textEdit, &QPlainTextEdit::textChanged,
     this, &MainWindow::onTextChanged);

        // Connect cursor position for status bar
  connect(textEdit, &QPlainTextEdit::cursorPositionChanged,
   this, &MainWindow::updateCursorPosition);

        updateTabTitle(doc);
    updateWindowTitle();

        // Apply theme to this editor
        textEdit->applyEditorTheme(currentTheme);

        // Set theme for highlighter
        HybridHighlighter* highlighter = getHighlighterForTextEdit(textEdit);
      if (highlighter) {
     bool isDark = (currentTheme == Settings::Dark || currentTheme == Settings::Hacker);
      highlighter->setTheme(isDark);
        }

      // Update status bar file type
    if (statusFileType) {
            if (isCodeFile(doc)) {
  statusFileType->setText("  C++  ");
       } else {
    statusFileType->setText("Plain Text  ");
 }
    }

        // Update cursor info
    updateCursorPosition();

        // Sync open editors list
   syncOpenEditorsList();
    }
}

// ── Highlighter ─────────────────────────────────────────────────
void MainWindow::applyHybridHighlighter(QPlainTextEdit* textEdit) {
    if (!textEdit) return;

    HybridHighlighter* highlighter = new HybridHighlighter(textEdit->document(), spellChecker);

    highlighter->setSyntaxHighlightingEnabled(isSyntaxHighlightingEnabled);
    highlighter->setSpellCheckEnabled(isSpellCheckEnabled);

    Document* doc = textEdit->property("document").value<Document*>();
    if (doc && isCodeFile(doc)) {
      highlighter->setForceCodeMode(true);
    }

    textEdit->setProperty("highlighter", QVariant::fromValue(static_cast<QObject*>(highlighter)));
}

bool MainWindow::isCodeFile(Document* doc) const {
    if (!doc || !doc->hasFilePath()) return false;

    QString filePath = doc->getFilePath();
    return filePath.endsWith(".cpp") ||
        filePath.endsWith(".h") ||
        filePath.endsWith(".hpp") ||
        filePath.endsWith(".c") ||
        filePath.endsWith(".cc") ||
        filePath.endsWith(".cxx") ||
        filePath.endsWith(".hxx") ||
     filePath.endsWith(".inl");
}

HybridHighlighter* MainWindow::getHighlighterForTextEdit(QPlainTextEdit* textEdit) const {
    if (!textEdit) return nullptr;

    QObject* obj = textEdit->property("highlighter").value<QObject*>();
    return qobject_cast<HybridHighlighter*>(obj);
}

void MainWindow::updateHighlighterSettings(QPlainTextEdit* textEdit) {
    HybridHighlighter* highlighter = getHighlighterForTextEdit(textEdit);
 if (highlighter) {
   highlighter->setSyntaxHighlightingEnabled(isSyntaxHighlightingEnabled);
     highlighter->setSpellCheckEnabled(isSpellCheckEnabled);
    }
}

// ── Toggle Features ─────────────────────────────────────────────
void MainWindow::onToggleSyntaxHighlighting() {
    isSyntaxHighlightingEnabled = !isSyntaxHighlightingEnabled;

    for (int i = 0; i < tabWidget->count(); ++i) {
      QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            updateHighlighterSettings(textEdit);
        }
    }

    QString status = isSyntaxHighlightingEnabled
        ? "Syntax highlighting enabled"
        : "Syntax highlighting disabled";
    statusBar()->showMessage(status, 3000);
}

void MainWindow::onToggleSpellCheck() {
    isSpellCheckEnabled = !isSpellCheckEnabled;

    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            updateHighlighterSettings(textEdit);
        }
    }

    QString status = isSpellCheckEnabled
        ? "Spell check enabled"
        : "Spell check disabled";
    statusBar()->showMessage(status, 3000);
}

// ── Context Menu ────────────────────────────────────────────────
void MainWindow::showContextMenu(const QPoint& pos) {
    QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(sender());
    if (!textEdit) return;

    QMenu* menu = textEdit->createStandardContextMenu();

    if (isSpellCheckEnabled && spellChecker && spellChecker->isValid()) {
        QTextCursor cursor = textEdit->cursorForPosition(pos);
    cursor.select(QTextCursor::WordUnderCursor);
        QString word = cursor.selectedText();

        if (!word.isEmpty() && !spellChecker->check(word)) {
       menu->addSeparator();
       QAction* title = menu->addAction("Suggestions:");
            title->setEnabled(false);

            QStringList suggestions = spellChecker->suggest(word);

          if (suggestions.isEmpty()) {
        QAction* noSugg = menu->addAction("(No suggestions)");
          noSugg->setEnabled(false);
       }
  else {
      for (const QString& sugg : suggestions) {
 QAction* action = menu->addAction(sugg);
        connect(action, &QAction::triggered, this, [textEdit, cursor, sugg]() mutable {
      textEdit->setTextCursor(cursor);
        cursor.beginEditBlock();
    cursor.insertText(sugg);
      cursor.endEditBlock();
        });
 }
      }
 }
    }

    menu->exec(textEdit->mapToGlobal(pos));
    delete menu;
}

// ═══════════════════════════════════════════════════════════════════
//  Settings
// ═══════════════════════════════════════════════════════════════════
void MainWindow::onOpenSettings() {
    Settings settingsDialog(this);
    connect(&settingsDialog, &Settings::settingsChanged, this, [&]() {
        applySettings(&settingsDialog);
        });
    settingsDialog.exec();
}

void MainWindow::applySettings(Settings* s) {
    if (!s) return;

    QFont font = s->getEditorFont();
    QColor color = s->getEditorColor();
    Settings::AppTheme theme = s->getSelectedTheme();

    if (theme == Settings::System) {
        bool isDark = Settings::isSystemDarkMode();
        theme = isDark ? Settings::Dark : Settings::Light;
    }

    currentTheme = theme;

    // Apply centralized theme to the entire application
    applyThemeToAllComponents(theme);

    // Apply font to all open editors
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            textEdit->setFont(font);

      HybridHighlighter* highlighter = getHighlighterForTextEdit(textEdit);
   if (highlighter) {
             bool isDark = (theme == Settings::Dark || theme == Settings::Hacker);
         highlighter->setTheme(isDark);
      }
        }
 }
}

// ═══════════════════════════════════════════════════════════════════
//  Theme Management (centralized via ThemeManager)
// ═══════════════════════════════════════════════════════════════════
void MainWindow::applyThemeToAllComponents(Settings::AppTheme theme) {
    // 1. Application-wide stylesheet
    applyThemeToApplication(theme);

  // 2. All open editors
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
            applyThemeToTextEdit(textEdit, theme);
 }
    }

    // 3. Toolbar
    if (mainToolBar) {
        mainToolBar->setStyleSheet(ThemeManager::getToolBarStyleSheet(theme));
    }

    // 4. Project explorer
    if (projectDock && projectTree) {
    projectDock->setStyleSheet(ThemeManager::getProjectExplorerStyleSheet(theme));
    }

    // 5. Output window
    if (outputWindow) {
        outputWindow->applyTheme(theme);
    }

    // 6. AI chat widget
    if (chatWidget) {
   chatWidget->applyTheme(theme);
    }

    // 7. Status bar accent for build state
    auto colors = ThemeManager::getColors(theme);
    if (statusBuildState) {
 statusBuildState->setStyleSheet(
     QString("padding: 2px 8px; font-weight: 600; color: %1;")
        .arg(colors.accent));
    }
}

void MainWindow::applyThemeToApplication(Settings::AppTheme theme) {
    this->setStyleSheet(ThemeManager::getApplicationStyleSheet(theme));
}

void MainWindow::applySystemTheme() {
    Settings s(this);
    Settings::AppTheme theme = s.getSelectedTheme();

    if (theme == Settings::System) {
        bool isDark = Settings::isSystemDarkMode();
        theme = isDark ? Settings::Dark : Settings::Light;
  }

    currentTheme = theme;
    applyThemeToAllComponents(theme);
    applySettings(&s);
}

void MainWindow::applyThemeToTextEdit(
    QPlainTextEdit* textEdit,
    Settings::AppTheme theme
) {
    // If it's a CodeEditorWidget, use its built-in theming
    CodeEditorWidget* codeEdit = qobject_cast<CodeEditorWidget*>(textEdit);
    if (codeEdit) {
        codeEdit->applyEditorTheme(theme);
    } else {
        // Fallback for plain QPlainTextEdit
        textEdit->setStyleSheet(ThemeManager::getEditorStyleSheet(theme));
}
}

// ═══════════════════════════════════════════════════════════════════
//  Document management
// ═══════════════════════════════════════════════════════════════════
void MainWindow::onDocumentClosed(Document* doc) {
    for (int i = 0; i < tabWidget->count(); ++i) {
  QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (textEdit) {
    Document* tabDoc = textEdit->property("document").value<Document*>();
            if (tabDoc == doc) {
   tabWidget->removeTab(i);
           textEdit->deleteLater();
     break;
     }
    }
    }
    updateWindowTitle();
    syncOpenEditorsList();
}

void MainWindow::onDocumentSaved(Document* doc) {
    if (doc) updateTabTitle(doc);
    updateWindowTitle();
    syncOpenEditorsList();
}

void MainWindow::onCurrentDocumentChanged(Document* doc) {
    if (doc) {
        for (int i = 0; i < tabWidget->count(); ++i) {
            QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
            if (textEdit) {
     Document* tabDoc = textEdit->property("document").value<Document*>();
       if (tabDoc == doc) {
          tabWidget->setCurrentIndex(i);
    break;
            }
            }
        }
    }
    updateWindowTitle();
}

void MainWindow::onTextChanged() {
    QPlainTextEdit* textEdit = getCurrentTextEdit();
    if (textEdit) {
        Document* doc = textEdit->property("document").value<Document*>();
        if (doc) {
   doc->setContent(textEdit->toPlainText());
    updateTabTitle(doc);
            updateWindowTitle();
        }
    }
}

void MainWindow::onTabChanged(int index) {
    if (index >= 0 && index < tabWidget->count()) {
      QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
        if (textEdit) {
       Document* doc = textEdit->property("document").value<Document*>();
         if (doc) {
 textEditor->switchToDocument(doc);

        // Update status bar file type
          if (statusFileType) {
            statusFileType->setText(isCodeFile(doc) ? "  C++  " : "  Plain Text  ");
      }
     }
  }
    }
    updateCursorPosition();
    syncOpenEditorsList();
}

void MainWindow::onTabCloseRequested(int index) {
    if (index >= 0 && index < tabWidget->count()) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
        if (textEdit) {
       Document* doc = textEdit->property("document").value<Document*>();
            if (doc) onFileClose();
        }
    }
}

QPlainTextEdit* MainWindow::getCurrentTextEdit() const {
    int index = tabWidget->currentIndex();
    if (index >= 0 && index < tabWidget->count())
      return qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
    return nullptr;
}

QPlainTextEdit* MainWindow::getTextEditForDocument(Document* doc) const {
    if (!doc) return nullptr;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
    if (textEdit) {
         Document* tabDoc = textEdit->property("document").value<Document*>();
            if (tabDoc == doc) return textEdit;
}
    }
    return nullptr;
}

void MainWindow::updateTabTitle(Document* doc) {
    if (!doc) return;
    QPlainTextEdit* textEdit = getTextEditForDocument(doc);
    if (textEdit) {
   int index = tabWidget->indexOf(textEdit);
        if (index >= 0) {
            QString title = doc->getFileName();
            if (doc->getIsModified()) title = "● " + title;
            tabWidget->setTabText(index, title);
        }
    }
}

void MainWindow::updateWindowTitle() {
    Document* doc = textEditor->getCurrentDocument();
    if (doc) {
        QString title = doc->getFileName();
        if (doc->getIsModified()) title = "● " + title;
        title += " — UITPad";
        setWindowTitle(title);
    }
    else {
        setWindowTitle("UITPad");
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Status bar: cursor position update
// ═══════════════════════════════════════════════════════════════════
void MainWindow::updateCursorPosition() {
    QPlainTextEdit* textEdit = getCurrentTextEdit();
    if (textEdit && statusLineCol) {
        QTextCursor cursor = textEdit->textCursor();
     int line = cursor.blockNumber() + 1;
  int col = cursor.columnNumber() + 1;
        statusLineCol->setText(QString("  Ln %1, Col %2  ").arg(line).arg(col));
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Project Explorer
// ═══════════════════════════════════════════════════════════════════
void MainWindow::toggleProjectExplorer() {
    if (!projectDock) return;

    if (projectDock->isVisible()) {
        projectDock->hide();
    } else {
 projectDock->show();
    }
}

void MainWindow::onOpenFolder() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Open Folder",
        QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirPath.isEmpty()) return;

updateProjectExplorerRoot(dirPath);
    statusBar()->showMessage("Opened folder: " + dirPath, 3000);
}

void MainWindow::onProjectFileClicked(const QModelIndex &index) {
    if (!fileSystemModel || !projectFilterProxy) return;

    QModelIndex sourceIndex = projectFilterProxy->mapToSource(index);
    QString filePath = fileSystemModel->filePath(sourceIndex);
    QFileInfo fileInfo(filePath);

    if (fileInfo.isFile()) {
   Document* doc = textEditor->openDocument(filePath);
        if (!doc) {
            QMessageBox::warning(this, "Error", "Cannot open file.");
        }
    }
}

void MainWindow::updateProjectExplorerRoot(const QString &filePath) {
    if (!fileSystemModel || !projectTree || !projectFilterProxy) return;
  QFileInfo fileInfo(filePath);
    QString dirPath = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.absolutePath();

    // If we already have this directory as root, skip
    if (explorerHasFolder) {
        QString currentRoot = fileSystemModel->rootPath();
 if (!currentRoot.isEmpty() && dirPath.startsWith(currentRoot)) {
     return;
    }
    }

    fileSystemModel->setRootPath(dirPath);
    projectTree->setRootIndex(projectFilterProxy->mapFromSource(fileSystemModel->index(dirPath)));

    // Show the tree and hide placeholder
    if (!explorerHasFolder) {
        if (placeholderWidget) placeholderWidget->hide();
        projectTree->show();
        explorerHasFolder = true;
    }

    // Update folder section title
    QDir dir(dirPath);
    if (folderTitleLabel) {
        folderTitleLabel->setText(dir.dirName().toUpper());
    }
    projectDock->setWindowTitle("EXPLORER");
}

// ═══════════════════════════════════════════════════════════════════
//  Open Editors Section
// ═══════════════════════════════════════════════════════════════════
void MainWindow::syncOpenEditorsList() {
    if (!openEditorsList) return;

    // Remember current selection
    int currentTabIdx = tabWidget->currentIndex();

 openEditorsList->clear();

    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
   if (!textEdit) continue;

      Document* doc = textEdit->property("document").value<Document*>();
        if (!doc) continue;

        // Create a widget for each item: [x] icon filename
        QWidget* itemWidget = new QWidget();
        itemWidget->setObjectName("openEditorItem");
        QHBoxLayout* layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(4, 1, 4, 1);
        layout->setSpacing(4);

        // Close button
        QPushButton* closeBtn = new QPushButton("×");
        closeBtn->setObjectName("editorCloseBtn");
closeBtn->setFixedSize(16, 16);
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setFlat(true);
  int tabIndex = i;
        connect(closeBtn, &QPushButton::clicked, this, [this, tabIndex]() {
     onOpenEditorCloseClicked(tabIndex);
        });

        // File name label
 QString fileName = doc->getFileName();
        if (doc->getIsModified()) fileName = "● " + fileName;
        QLabel* nameLabel = new QLabel(fileName);
        nameLabel->setStyleSheet("border: none; background: transparent; font-size: 12px;");

layout->addWidget(closeBtn);
   layout->addWidget(nameLabel);
  layout->addStretch();

        QListWidgetItem* listItem = new QListWidgetItem();
        listItem->setData(Qt::UserRole, i);  // Store tab index
        listItem->setSizeHint(QSize(0, 22));
        openEditorsList->addItem(listItem);
 openEditorsList->setItemWidget(listItem, itemWidget);

        // Highlight current tab
        if (i == currentTabIdx) {
          listItem->setSelected(true);
        }
    }

    // Adjust height based on item count
    int itemCount = openEditorsList->count();
    int listHeight = qMin(itemCount * 22, 150);
    openEditorsList->setFixedHeight(qMax(listHeight, 0));
}

void MainWindow::onOpenEditorItemClicked(QListWidgetItem *item) {
    if (!item) return;
    int tabIndex = item->data(Qt::UserRole).toInt();
    if (tabIndex >= 0 && tabIndex < tabWidget->count()) {
        tabWidget->setCurrentIndex(tabIndex);
    }
}

void MainWindow::onOpenEditorCloseClicked(int tabIndex) {
    if (tabIndex >= 0 && tabIndex < tabWidget->count()) {
        QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(tabWidget->widget(tabIndex));
        if (textEdit) {
        Document* doc = textEdit->property("document").value<Document*>();
     if (doc) {
                // Switch to this tab first
     tabWidget->setCurrentIndex(tabIndex);
      onFileClose();
            }
      }
    }
}

void MainWindow::toggleOpenEditorsSection() {
  openEditorsSectionExpanded = !openEditorsSectionExpanded;
    if (openEditorsList) {
      openEditorsList->setVisible(openEditorsSectionExpanded);
    }
    if (openEditorsArrow) {
  openEditorsArrow->setText(openEditorsSectionExpanded ? "▾" : "▸");
    }
}

void MainWindow::toggleFolderSection() {
  folderSectionExpanded = !folderSectionExpanded;
    if (explorerHasFolder) {
     projectTree->setVisible(folderSectionExpanded);
    } else {
        if (placeholderWidget) placeholderWidget->setVisible(folderSectionExpanded);
    }
    if (folderArrow) {
     folderArrow->setText(folderSectionExpanded ? "▾" : "▸");
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Compiler Actions
// ═══════════════════════════════════════════════════════════════════
void MainWindow::onCompile() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc) {
        QMessageBox::warning(this, "Error", "No file to compile.");
   return;
    }

  // Save before compiling
    if (doc->getIsModified()) {
        onFileSave();
    }

    if (!doc->hasFilePath()) {
        QMessageBox::warning(this, "Error", "Please save the file before compiling.");
        return;
    }

    if (!compilerManager->isCompilerAvailable()) {
        compilerManager->detectCompiler();
  if (!compilerManager->isCompilerAvailable()) {
   QMessageBox::warning(this, "Compiler Not Found",
      "No C++ compiler was detected on your system.\n"
     "Please install GCC (MinGW), MSVC, or Clang.");
   return;
     }
    }

    compilerManager->compileFile(doc->getFilePath());
}

void MainWindow::onCompileAndRun() {
 Document* doc = textEditor->getCurrentDocument();
    if (!doc) {
        QMessageBox::warning(this, "Error", "No file to compile.");
  return;
    }

    if (doc->getIsModified()) {
    onFileSave();
    }

    if (!doc->hasFilePath()) {
        QMessageBox::warning(this, "Error", "Please save the file before compiling.");
        return;
    }

    if (!compilerManager->isCompilerAvailable()) {
        compilerManager->detectCompiler();
        if (!compilerManager->isCompilerAvailable()) {
            QMessageBox::warning(this, "Compiler Not Found",
 "No C++ compiler was detected on your system.\n"
         "Please install GCC (MinGW), MSVC, or Clang.");
  return;
  }
    }

    compilerManager->compileAndRun(doc->getFilePath());
}

void MainWindow::onRun() {
    Document* doc = textEditor->getCurrentDocument();
    if (!doc || !doc->hasFilePath()) {
        QMessageBox::warning(this, "Error", "No file to run.");
        return;
    }

    QString filePath = doc->getFilePath();
    QString exePath = filePath;
    exePath.replace(QRegularExpression("\\.(cpp|c|cc|cxx)$"), ".exe");

    if (!QFileInfo::exists(exePath)) {
        QMessageBox::warning(this, "Error",
            "Executable not found. Please compile the file first.");
        return;
    }

    compilerManager->runExecutable(exePath);
}

void MainWindow::onStopCompilation() {
    if (compilerManager->isCompiling()) {
        compilerManager->stopCompilation();
        statusBar()->showMessage("Compilation stopped", 3000);
        if (statusBuildState) statusBuildState->setText("  Ready  ");
    }
}

void MainWindow::onStopExecution() {
    if (compilerManager->isRunning()) {
   compilerManager->stopExecution();
        statusBar()->showMessage("Execution stopped", 3000);
if (statusBuildState) statusBuildState->setText("  Ready  ");
    }
}

void MainWindow::onCompilerSettings() {
    QMessageBox::information(this, "Compiler Settings",
     QString("Compiler: %1\nPath: %2\nAvailable: %3")
    .arg(compilerManager->getCompilerType() == CompilerManager::GCC ? "GCC" :
              compilerManager->getCompilerType() == CompilerManager::MSVC ? "MSVC" :
       compilerManager->getCompilerType() == CompilerManager::CLANG ? "Clang" : "Auto")
            .arg(compilerManager->getCompilerPath())
          .arg(compilerManager->isCompilerAvailable() ? "Yes" : "No"));
}

// ═══════════════════════════════════════════════════════════════════
//  AI Actions
// ═══════════════════════════════════════════════════════════════════
void MainWindow::onExplainCode() {
    QPlainTextEdit* textEdit = getCurrentTextEdit();
    if (!textEdit) return;

    QString selectedText = textEdit->textCursor().selectedText();
    if (selectedText.isEmpty()) {
        QMessageBox::information(this, "AI - Explain Code",
          "Please select some code to explain.");
return;
    }

    if (!aiAssistant || !aiAssistant->isConfigured()) {
        QMessageBox::warning(this, "AI Not Configured",
  "Please configure the AI API key in AI > AI Settings...");
    return;
    }

  chatWidget->show();
    chatWidget->raise();
    aiAssistant->explainCode(selectedText);
}

void MainWindow::onCompleteCode() {
    QPlainTextEdit* textEdit = getCurrentTextEdit();
 if (!textEdit) return;

    QString context = textEdit->toPlainText();
 int cursorPos = textEdit->textCursor().position();
    QString contextBeforeCursor = context.left(cursorPos);

    if (contextBeforeCursor.trimmed().isEmpty()) {
        QMessageBox::information(this, "AI - Complete Code",
            "Please write some code first for completion context.");
        return;
  }

    if (!aiAssistant || !aiAssistant->isConfigured()) {
        QMessageBox::warning(this, "AI Not Configured",
        "Please configure the AI API key in AI > AI Settings...");
 return;
    }

    chatWidget->show();
    chatWidget->raise();
    aiAssistant->completeCode(contextBeforeCursor);
}

void MainWindow::onGenerateFromComment() {
    QPlainTextEdit* textEdit = getCurrentTextEdit();
    if (!textEdit) return;

    QTextCursor cursor = textEdit->textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    QString line = cursor.selectedText().trimmed();

    if (line.isEmpty()) {
        QMessageBox::information(this, "AI - Generate from Comment",
  "Please place the cursor on a comment line.");
        return;
    }

    if (!aiAssistant || !aiAssistant->isConfigured()) {
        QMessageBox::warning(this, "AI Not Configured",
      "Please configure the AI API key in AI > AI Settings...");
    return;
    }

    chatWidget->show();
    chatWidget->raise();
    aiAssistant->generateCode(line);
}

void MainWindow::onAISettings() {
    if (!aiAssistant) return;

    AISettingsDialog dialog(aiAssistant, this);
    dialog.exec();
}

void MainWindow::onAIError(const QString &error) {
    QMessageBox::warning(this, "AI Error", error);
}

void MainWindow::toggleChatWidget() {
    if (!chatWidget) return;

    if (chatWidget->isVisible()) {
        chatWidget->hide();
    } else {
        chatWidget->show();
        chatWidget->raise();
    }
}

void MainWindow::onCodeInsertRequested(const QString &code) {
    QPlainTextEdit* textEdit = getCurrentTextEdit();
    if (!textEdit) {
        QMessageBox::information(this, "Insert Code",
     "No editor tab open to insert code into.");
     return;
    }

    QTextCursor cursor = textEdit->textCursor();
    cursor.beginEditBlock();
    cursor.insertText(code);
    cursor.endEditBlock();
    textEdit->setTextCursor(cursor);

    statusBar()->showMessage("Code inserted", 3000);
}

void MainWindow::onCodeCopyRequested(const QString &code) {
    QApplication::clipboard()->setText(code);
    statusBar()->showMessage("Code copied to clipboard", 3000);
}

// ═══════════════════════════════════════════════════════════════════
//  Event Filter
// ═══════════════════════════════════════════════════════════════════
bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    // Handle section header clicks for collapsible sections
    if (event->type() == QEvent::MouseButtonRelease) {
  if (obj == openEditorsHeader) {
     toggleOpenEditorsSection();
   return true;
     }
  if (obj == folderHeader) {
            toggleFolderSection();
         return true;
        }
  }
    return QMainWindow::eventFilter(obj, event);
}