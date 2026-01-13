#include "CompilerManager.h"
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QDebug>
#include <QTemporaryFile>
#include <QTextStream>
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <windows.h>  // For CREATE_NEW_CONSOLE
#endif

CompilerManager::CompilerManager(QObject* parent)
    : QObject(parent),
      compilerType(AUTO),
      compilationStandard(Cpp17),
      optimizationLevel(0),
      warningLevel(1),
      compilerAvailable(false),
      compiling(false),
      running(false),
      compileAndRunMode(false)
{
    compileProcess = new QProcess(this);
    runProcess = new QProcess(this);

    // Connect compile process signals
    connect(compileProcess, &QProcess::readyReadStandardOutput,
            this, &CompilerManager::onCompileProcessReadyReadStdOut);
    connect(compileProcess, &QProcess::readyReadStandardError,
            this, &CompilerManager::onCompileProcessReadyReadStdErr);
    connect(compileProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CompilerManager::onCompileProcessFinished);

    // Connect run process signals
    connect(runProcess, &QProcess::readyReadStandardOutput,
            this, &CompilerManager::onRunProcessReadyReadStdOut);
    connect(runProcess, &QProcess::readyReadStandardError,
            this, &CompilerManager::onRunProcessReadyReadStdErr);
    connect(runProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CompilerManager::onRunProcessFinished);

    // Auto-detect compiler on startup
    detectCompiler();
}

CompilerManager::~CompilerManager()
{
    if (compileProcess->state() != QProcess::NotRunning) {
        compileProcess->kill();
        compileProcess->waitForFinished();
    }
    if (runProcess->state() != QProcess::NotRunning) {
        runProcess->kill();
        runProcess->waitForFinished();
    }
    
    // Clean up temporary file
    cleanupTempFile();
}

void CompilerManager::cleanupTempFile()
{
    if (!tempSourceFile.isEmpty() && QFile::exists(tempSourceFile)) {
        QFile::remove(tempSourceFile);
        tempSourceFile.clear();
    }
}

bool CompilerManager::detectCompiler()
{
    if (compilerType == AUTO) {
        // Try to detect in order: GCC, Clang, MSVC
        if (detectGCC()) {
            compilerType = GCC;
            compilerAvailable = true;
            emit compilerDetected(GCC, compilerPath);
            qDebug() << "Detected GCC at:" << compilerPath;
            return true;
        }
        else if (detectClang()) {
            compilerType = CLANG;
            compilerAvailable = true;
            emit compilerDetected(CLANG, compilerPath);
            qDebug() << "Detected Clang at:" << compilerPath;
            return true;
        }
        else if (detectMSVC()) {
            compilerType = MSVC;
            compilerAvailable = true;
            emit compilerDetected(MSVC, compilerPath);
            qDebug() << "Detected MSVC at:" << compilerPath;
            return true;
        }
        
        emit compilerNotFound();
        return false;
    }
    return compilerAvailable;
}

bool CompilerManager::detectGCC()
{
#ifdef Q_OS_WIN
    QString gccPath = findExecutableInPath("g++.exe");
    if (gccPath.isEmpty()) {
        // Try common MinGW locations
        QStringList possiblePaths = {
            "C:/msys64/mingw64/bin/g++.exe",
            "C:/msys64/ucrt64/bin/g++.exe",
            "C:/mingw64/bin/g++.exe",
            "C:/MinGW/bin/g++.exe",
            "C:/TDM-GCC-64/bin/g++.exe"
        };
        for (const QString& path : possiblePaths) {
            if (QFile::exists(path)) {
                compilerPath = path;
                return true;
            }
        }
    }
#else
    QString gccPath = findExecutableInPath("g++");
#endif
    
    if (!gccPath.isEmpty()) {
        compilerPath = gccPath;
        return true;
    }
    return false;
}

bool CompilerManager::detectClang()
{
#ifdef Q_OS_WIN
    QString clangPath = findExecutableInPath("clang++.exe");
#else
    QString clangPath = findExecutableInPath("clang++");
#endif
    
    if (!clangPath.isEmpty()) {
        compilerPath = clangPath;
        return true;
    }
    return false;
}

bool CompilerManager::detectMSVC()
{
#ifdef Q_OS_WIN
    // MSVC is more complex to detect, usually requires vcvars setup
    QString clPath = findExecutableInPath("cl.exe");
    if (!clPath.isEmpty()) {
        compilerPath = clPath;
        return true;
    }
#endif
    return false;
}

QString CompilerManager::findExecutableInPath(const QString& executable)
{
    QString path = QStandardPaths::findExecutable(executable);
    return path;
}

void CompilerManager::setCompilerType(CompilerType type)
{
    compilerType = type;
    detectCompiler();
}

void CompilerManager::setCompilationStandard(CompilationStandard standard)
{
    compilationStandard = standard;
}

void CompilerManager::setOptimizationLevel(int level)
{
    optimizationLevel = qBound(0, level, 3);
}

void CompilerManager::setWarningLevel(int level)
{
    warningLevel = qBound(0, level, 3);
}

void CompilerManager::setOutputDirectory(const QString& dir)
{
    outputDirectory = dir;
}

void CompilerManager::setAdditionalFlags(const QStringList& flags)
{
    additionalFlags = flags;
}

QString CompilerManager::getStandardFlag() const
{
    switch (compilationStandard) {
        case Cpp11: return "-std=c++11";
        case Cpp14: return "-std=c++14";
        case Cpp17: return "-std=c++17";
        case Cpp20: return "-std=c++20";
        case Cpp23: return "-std=c++23";
        default: return "-std=c++17";
    }
}

QString CompilerManager::getOptimizationFlag() const
{
    return QString("-O%1").arg(optimizationLevel);
}

QStringList CompilerManager::getWarningFlags() const
{
    QStringList flags;
    if (warningLevel >= 1) flags << "-Wall";
    if (warningLevel >= 2) flags << "-Wextra";
    if (warningLevel >= 3) flags << "-Wpedantic";
    return flags;
}

QString CompilerManager::generateOutputFileName(const QString& sourceFile) const
{
    QFileInfo fileInfo(sourceFile);
    QString baseName = fileInfo.completeBaseName();
    QString outputDir = outputDirectory.isEmpty() ? fileInfo.absolutePath() : outputDirectory;
    
#ifdef Q_OS_WIN
    return QDir(outputDir).filePath(baseName + ".exe");
#else
    return QDir(outputDir).filePath(baseName);
#endif
}

QStringList CompilerManager::buildCompileCommand(const QString& sourceFile, const QString& outputFile)
{
    QStringList args;
    
    args << sourceFile;
    args << "-o" << outputFile;
    args << getStandardFlag();
    args << getOptimizationFlag();
    args << getWarningFlags();
    
    // Add static linking flags for MinGW/GCC
    if (compilerType == GCC || compilerType == CLANG) {
        args << "-static";
        args << "-static-libgcc";
        args << "-static-libstdc++";
    }
    
    args << additionalFlags;
    
    return args;
}

// Check if a file has a standard C++ extension
bool CompilerManager::isStandardCppFile(const QString& filePath) const
{
    QString ext = QFileInfo(filePath).suffix().toLower();
    return ext == "cpp" || ext == "cxx" || ext == "cc" || 
           ext == "c" || ext == "c++" || ext == "hpp" || ext == "h";
}

// Detect if a line looks like C++ code
bool CompilerManager::isCodeLine(const QString& line) const
{
    QString trimmed = line.trimmed();
    
    // Empty lines are kept (might be part of code formatting)
    if (trimmed.isEmpty()) {
        return true;  // Keep empty lines between code
    }
    
    // Preprocessor directives
    if (trimmed.startsWith("#include") || trimmed.startsWith("#define") ||
        trimmed.startsWith("#ifdef") || trimmed.startsWith("#ifndef") ||
        trimmed.startsWith("#endif") || trimmed.startsWith("#pragma") ||
        trimmed.startsWith("#else") || trimmed.startsWith("#elif") ||
        trimmed.startsWith("#undef") || trimmed.startsWith("#error")) {
        return true;
    }
    
    // C++ comments
    if (trimmed.startsWith("//") || trimmed.startsWith("/*") || 
        trimmed.startsWith("*") || trimmed.endsWith("*/")) {
        return true;
    }
    
    // Braces
    if (trimmed == "{" || trimmed == "}" || trimmed == "};" || 
        trimmed.endsWith("{") || trimmed.startsWith("}")) {
        return true;
    }
    
    // Lines ending with semicolon (statements)
    if (trimmed.endsWith(";")) {
        return true;
    }
    
    // Function declarations/definitions
    if (QRegularExpression("^\\w+\\s+\\w+\\s*\\([^)]*\\)\\s*\\{?$").match(trimmed).hasMatch()) {
        return true;
    }
    
    // Class/struct/enum declarations
    if (QRegularExpression("^(class|struct|enum|namespace|template|typedef|using)\\s+").match(trimmed).hasMatch()) {
        return true;
    }
    
    // Control flow keywords
    if (QRegularExpression("^(if|else|while|for|do|switch|case|default|return|break|continue|try|catch|throw)\\b").match(trimmed).hasMatch()) {
        return true;
    }
    
    // Variable declarations with common types
    if (QRegularExpression("^(int|float|double|char|bool|void|auto|const|static|unsigned|long|short|signed|string|QString|std::\\w+)\\s+\\w+").match(trimmed).hasMatch()) {
        return true;
    }
    
    // Access specifiers
    if (QRegularExpression("^(public|private|protected)\\s*:").match(trimmed).hasMatch()) {
        return true;
    }
    
    // Contains scope resolution operator
    if (trimmed.contains("::")) {
        return true;
    }
    
    // Contains common C++ operators in context
    if (trimmed.contains("->") || trimmed.contains("<<") || trimmed.contains(">>")) {
        return true;
    }
    
    // Assignment or comparison
    if (QRegularExpression("\\w+\\s*[=!<>]=?\\s*").match(trimmed).hasMatch() && 
        (trimmed.contains("(") || trimmed.contains(";"))) {
        return true;
    }
    
    return false;
}

// Extract only code lines from content
QString CompilerManager::extractCodeFromContent(const QString& content) const
{
    QStringList lines = content.split('\n');
    QStringList codeLines;
    bool inCodeBlock = false;
    int consecutiveCodeLines = 0;
    int consecutiveTextLines = 0;
    
    // First pass: identify code regions
    QList<bool> isCode;
    for (const QString& line : lines) {
        isCode.append(isCodeLine(line));
    }
    
    // Second pass: smooth out the detection (isolated text lines within code are probably code)
    for (int i = 0; i < lines.size(); ++i) {
        QString trimmed = lines[i].trimmed();
        
        // Check context: if surrounded by code lines, treat as code
        bool prevIsCode = (i > 0) ? isCode[i-1] : false;
        bool nextIsCode = (i < lines.size() - 1) ? isCode[i+1] : false;
        
        if (!isCode[i] && prevIsCode && nextIsCode && !trimmed.isEmpty()) {
            // Isolated non-code line between code - might be a comment or keep it
            // Only exclude if it looks clearly like natural language (multiple words, no special chars)
            if (!trimmed.contains(QRegularExpression("[;{}()\\[\\]<>]")) && 
                trimmed.split(' ').size() > 3) {
                // Looks like natural text, skip it
                continue;
            }
        }
        
        if (isCode[i] || trimmed.isEmpty()) {
            codeLines.append(lines[i]);
        }
    }
    
    // Remove leading empty lines
    while (!codeLines.isEmpty() && codeLines.first().trimmed().isEmpty()) {
        codeLines.removeFirst();
    }
    
    // Remove trailing empty lines
    while (!codeLines.isEmpty() && codeLines.last().trimmed().isEmpty()) {
        codeLines.removeLast();
    }
    
    return codeLines.join('\n');
}

// Create a temporary .cpp file with extracted code
QString CompilerManager::createTempSourceFile(const QString& originalFile, const QString& content)
{
    // Clean up any previous temp file
    cleanupTempFile();
    
    QFileInfo fileInfo(originalFile);
    QString tempDir = fileInfo.absolutePath();
    QString baseName = fileInfo.completeBaseName();
    
    // Create temp file path
    tempSourceFile = QDir(tempDir).filePath(baseName + "_temp.cpp");
    
    // Extract code from content
    QString codeContent = extractCodeFromContent(content);
    
    // Write to temp file
    QFile file(tempSourceFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << codeContent;
        file.close();
        
        emit compilationOutput(">> Extracted code to temporary file: " + tempSourceFile + "\n");
        emit compilationOutput(">> ----------------------------------------\n");
        
        return tempSourceFile;
    }
    
    return QString();
}

void CompilerManager::compileFile(const QString& sourceFile, const QString& outputFile)
{
    if (!compilerAvailable) {
        emit compilationError("No compiler available. Please install g++ or clang++.");
        emit compilationFinished(false, -1);
        return;
    }

    if (compiling) {
        emit compilationError("Compilation already in progress.");
        return;
    }

    QString fileToCompile = sourceFile;
    QString output = outputFile.isEmpty() ? generateOutputFileName(sourceFile) : outputFile;
    
    // If not a standard C++ file, extract code and create temp file
    if (!isStandardCppFile(sourceFile)) {
        // Read the original file content
        QFile file(sourceFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = QTextStream(&file).readAll();
            file.close();
            
            QString tempFile = createTempSourceFile(sourceFile, content);
            if (!tempFile.isEmpty()) {
                fileToCompile = tempFile;
                // Update output to use original file's base name
                output = generateOutputFileName(sourceFile);
            } else {
                emit compilationError("Failed to create temporary source file.");
                emit compilationFinished(false, -1);
                return;
            }
        } else {
            emit compilationError("Failed to read source file: " + sourceFile);
            emit compilationFinished(false, -1);
            return;
        }
    }
    
    QStringList args = buildCompileCommand(fileToCompile, output);
    
    compiling = true;
    emit compilationStarted(sourceFile);
    
    QString command = compilerPath + " " + args.join(" ");
    emit compilationOutput(">> " + command + "\n");
    
    compileProcess->start(compilerPath, args);
}

void CompilerManager::compileAndRun(const QString& sourceFile, const QString& outputFile)
{
    pendingExecutable = outputFile.isEmpty() ? generateOutputFileName(sourceFile) : outputFile;
    compileAndRunMode = true;
    compileFile(sourceFile, pendingExecutable);
}

void CompilerManager::runExecutable(const QString& executablePath, const QStringList& arguments)
{
    Q_UNUSED(arguments);
    
    if (running) {
        emit executionError("Program already running.");
        return;
    }

    if (!QFile::exists(executablePath)) {
        emit executionError("Executable not found: " + executablePath);
        emit executionFinished(-1);
        return;
    }

    running = true;
    emit executionStarted(executablePath);
    emit executionOutput(">> Running in external console: " + executablePath + "\n");

#ifdef Q_OS_WIN
    // Run in a new console window on Windows using cmd /C start
    runProcess->setProgram("cmd.exe");
    runProcess->setArguments(QStringList() << "/C" << "start" << "cmd" << "/K" << executablePath);
    runProcess->start();
#else
    // On Linux/Mac, use a terminal emulator
    if (QFile::exists("/usr/bin/gnome-terminal")) {
        runProcess->start("gnome-terminal", QStringList() << "--" << executablePath);
    } else if (QFile::exists("/usr/bin/xterm")) {
        runProcess->start("xterm", QStringList() << "-hold" << "-e" << executablePath);
    } else if (QFile::exists("/usr/bin/konsole")) {
        runProcess->start("konsole", QStringList() << "-e" << executablePath);
    } else {
        // Fallback - won't support interactive input
        runProcess->start(executablePath);
    }
#endif
}

void CompilerManager::stopCompilation()
{
    if (compiling && compileProcess->state() != QProcess::NotRunning) {
        compileProcess->kill();
        emit compilationOutput("\n>> Compilation stopped by user.\n");
    }
}

void CompilerManager::stopExecution()
{
    if (running && runProcess->state() != QProcess::NotRunning) {
        runProcess->kill();
        emit executionOutput("\n>> Execution stopped by user.\n");
    }
}

void CompilerManager::onCompileProcessReadyReadStdOut()
{
    QString output = QString::fromLocal8Bit(compileProcess->readAllStandardOutput());
    emit compilationOutput(output);
}

void CompilerManager::onCompileProcessReadyReadStdErr()
{
    QString error = QString::fromLocal8Bit(compileProcess->readAllStandardError());
    emit compilationError(error);
}

void CompilerManager::onCompileProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    compiling = false;
    bool success = (exitCode == 0 && exitStatus == QProcess::NormalExit);
    
    if (success) {
        emit compilationOutput("\n>> Compilation successful!\n");
    } else {
        emit compilationOutput("\n>> Compilation failed with exit code " + QString::number(exitCode) + "\n");
    }
    
    // Clean up temp file after compilation
    cleanupTempFile();
    
    emit compilationFinished(success, exitCode);
    
    if (compileAndRunMode && success) {
        compileAndRunMode = false;
        runExecutable(pendingExecutable);
    } else {
        compileAndRunMode = false;
    }
}

void CompilerManager::onRunProcessReadyReadStdOut()
{
    QString output = QString::fromLocal8Bit(runProcess->readAllStandardOutput());
    emit executionOutput(output);
}

void CompilerManager::onRunProcessReadyReadStdErr()
{
    QString error = QString::fromLocal8Bit(runProcess->readAllStandardError());
    emit executionError(error);
}

void CompilerManager::onRunProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    running = false;
    Q_UNUSED(exitStatus);
    
    emit executionOutput("\n----------------------------------------\n");
    emit executionOutput(">> External console launched.\n");
    emit executionFinished(exitCode);
}