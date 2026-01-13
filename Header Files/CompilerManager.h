#ifndef COMPILERMANAGER_H
#define COMPILERMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class CompilerManager : public QObject
{
    Q_OBJECT

public:
    enum CompilerType {
        GCC,      // g++ (MinGW on Windows, GCC on Linux)
        MSVC,     // Microsoft Visual C++ (cl.exe)
        CLANG,    // Clang/LLVM
        AUTO      // Auto-detect available compiler
    };

    enum CompilationStandard {
        Cpp11,
        Cpp14,
        Cpp17,
        Cpp20,
        Cpp23
    };

    explicit CompilerManager(QObject* parent = nullptr);
    ~CompilerManager();

    // Compiler detection and configuration
    bool detectCompiler();
    void setCompilerType(CompilerType type);
    CompilerType getCompilerType() const { return compilerType; }
    QString getCompilerPath() const { return compilerPath; }
    bool isCompilerAvailable() const { return compilerAvailable; }

    // Compilation options
    void setCompilationStandard(CompilationStandard standard);
    void setOptimizationLevel(int level); // 0-3
    void setWarningLevel(int level);      // 0-3
    void setOutputDirectory(const QString& dir);
    void setAdditionalFlags(const QStringList& flags);

    // Compile and run
    void compileFile(const QString& sourceFile, const QString& outputFile = QString());
    void compileAndRun(const QString& sourceFile, const QString& outputFile = QString());
    void runExecutable(const QString& executablePath, const QStringList& arguments = QStringList());

    // Process control
    void stopCompilation();
    void stopExecution();
    bool isCompiling() const { return compiling; }
    bool isRunning() const { return running; }

signals:
    // Compilation signals
    void compilationStarted(const QString& sourceFile);
    void compilationOutput(const QString& output);
    void compilationError(const QString& error);
    void compilationFinished(bool success, int exitCode);

    // Execution signals
    void executionStarted(const QString& executable);
    void executionOutput(const QString& output);
    void executionError(const QString& error);
    void executionFinished(int exitCode);

    // General signals
    void compilerDetected(CompilerType type, const QString& path);
    void compilerNotFound();

private slots:
    void onCompileProcessReadyReadStdOut();
    void onCompileProcessReadyReadStdErr();
    void onCompileProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onRunProcessReadyReadStdOut();
    void onRunProcessReadyReadStdErr();
    void onRunProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    // Helper methods
    bool detectGCC();
    bool detectMSVC();
    bool detectClang();
    QString findExecutableInPath(const QString& executable);
    QStringList buildCompileCommand(const QString& sourceFile, const QString& outputFile);
    QString getStandardFlag() const;
    QString getOptimizationFlag() const;
    QStringList getWarningFlags() const;
    QString generateOutputFileName(const QString& sourceFile) const;

    // Member variables
    QProcess* compileProcess;
    QProcess* runProcess;
    CompilerType compilerType;
    CompilationStandard compilationStandard;
    QString compilerPath;
    QString outputDirectory;
    QStringList additionalFlags;
    int optimizationLevel;
    int warningLevel;
    bool compilerAvailable;
    bool compiling;
    bool running;
    bool compileAndRunMode;
    QString pendingExecutable;

    // Temporary file management
    QString tempSourceFile;

    // Helper methods for non-standard files
    bool isStandardCppFile(const QString& filePath) const;
    bool isCodeLine(const QString& line) const;
    QString extractCodeFromContent(const QString& content) const;
    QString createTempSourceFile(const QString& originalFile, const QString& content);
    void cleanupTempFile();
};

#endif // COMPILERMANAGER_H