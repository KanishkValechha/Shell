#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), process(new QProcess(this)) {
    QWidget *central = new QWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(central);
    
    outputEdit = new QTextEdit(this);
    outputEdit->setReadOnly(true);
    inputEdit = new QLineEdit(this);
    runButton = new QPushButton("Run", this);
    shellButton = new QPushButton("Start Shell", this);
    
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(inputEdit);
    hLayout->addWidget(runButton);
    
    vLayout->addWidget(outputEdit);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(shellButton);
    
    setCentralWidget(central);
    resize(600, 400);
    
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runCommand);
    connect(shellButton, &QPushButton::clicked, this, &MainWindow::launchShell);
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::processOutput);
}

void MainWindow::runCommand() {
    QString cmd = inputEdit->text().trimmed();
    if(cmd.isEmpty())
        return;
    outputEdit->append("Command: " + cmd);
    // Execute cmd.exe with /c prefix
    process->start("cmd.exe", QStringList() << "/c" << cmd);
    inputEdit->clear();
}

void MainWindow::processOutput() {
    QByteArray data = process->readAllStandardOutput();
    outputEdit->append(QString::fromLocal8Bit(data));
}

void MainWindow::launchShell() {
    QProcess *shellProc = new QProcess(this);
    shellProc->start("shell.exe");
    if (!shellProc->waitForStarted(3000)) {
        QMessageBox::critical(this, "Error", "Failed to launch shell.exe");
        shellProc->deleteLater();
    }
}
