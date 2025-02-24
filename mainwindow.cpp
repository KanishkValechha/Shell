#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFontDatabase>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Set up central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Create toolbar
    QToolBar *toolbar = addToolBar("Main Toolbar");
    QAction *clearAction = toolbar->addAction("Clear");
    QAction *aboutAction = toolbar->addAction("About");
    
    // Set up output display with monospace font
    outputEdit = new QTextEdit(this);
    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setPointSize(10);
    outputEdit->setFont(monoFont);
    outputEdit->setReadOnly(true);
    outputEdit->setStyleSheet("QTextEdit { background-color: #1E1E1E; color: #D4D4D4; }");
    
    // Set up command input with history
    inputEdit = new CommandLineEdit(this);
    inputEdit->setFont(monoFont);
    inputEdit->setStyleSheet("QLineEdit { background-color: #2D2D2D; color: #D4D4D4; padding: 5px; border: 1px solid #3D3D3D; }");
    
    // Set up buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    runButton = new QPushButton("Run", this);
    shellButton = new QPushButton("Start Shell", this);
    runButton->setStyleSheet("QPushButton { background-color: #0E639C; color: white; padding: 5px 15px; }");
    shellButton->setStyleSheet("QPushButton { background-color: #0E639C; color: white; padding: 5px 15px; }");

    // Layout setup
    mainLayout->addWidget(outputEdit);
    mainLayout->addWidget(inputEdit);
    buttonLayout->addWidget(runButton);
    buttonLayout->addWidget(shellButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // Status bar
    statusBar()->showMessage("Ready");

    // Process setup
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);

    // Connect signals
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runCommand);
    connect(shellButton, &QPushButton::clicked, this, &MainWindow::launchShell);
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::processOutput);
    connect(clearAction, &QAction::triggered, outputEdit, &QTextEdit::clear);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About Shell GUI",
            "Enhanced Shell GUI\nVersion 1.0\n\nA modern interface for command line operations.");
    });

    // Set window properties
    resize(800, 600);
    setMinimumSize(500, 400);
}

void MainWindow::runCommand() {
    QString cmd = inputEdit->text().trimmed();
    if (cmd.isEmpty()) return;

    outputEdit->append(QString("> %1").arg(cmd));
    process->start("cmd.exe", QStringList() << "/c" << cmd);
    inputEdit->addToHistory(cmd);
    inputEdit->clear();
}

void MainWindow::processOutput() {
    QString output = QString::fromLocal8Bit(process->readAll());
    outputEdit->append(output);
    statusBar()->showMessage("Command completed", 3000);
}

void MainWindow::launchShell() {
    QProcess::startDetached("shell.exe");
}
