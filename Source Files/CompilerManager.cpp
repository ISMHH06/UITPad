#include "CompilerManager.h"
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QDebug>

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

    QString output = outputFile.isEmpty() ? generateOutputFileName(sourceFile) : outputFile;
    QStringList args = buildCompileCommand(sourceFile, output);
    
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