// Source Files/MainWindow.cpp

#include "MainWindow.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Zone de texte
    textArea = new QPlainTextEdit(this);
    setCentralWidget(textArea);

    // Menu Fichier
    QMenu* fileMenu = menuBar()->addMenu("Fichier");
    fileMenu->addAction("Ouvrir", this, &MainWindow::onFileOpen);
    fileMenu->addAction("Sauvegarder", this, &MainWindow::onFileSave);
    fileMenu->addAction("Quitter", this, &QWidget::close);
}

void MainWindow::onFileOpen() {
    QString filename = QFileDialog::getOpenFileName(this, "Ouvrir");
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        textArea->setPlainText(in.readAll());
        file.close();
    }
}

void MainWindow::onFileSave() {
    QString filename = QFileDialog::getSaveFileName(this, "Sauvegarder");
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << textArea->toPlainText();
        file.close();
    }
}