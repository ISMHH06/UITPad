// Header Files/MainWindow.h

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QPlainTextEdit* textArea;

public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void onFileOpen();
    void onFileSave();
};

#endif