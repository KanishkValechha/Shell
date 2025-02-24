#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QProcess>
#include "commandlineedit.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private slots:
    void runCommand();
    void processOutput();
    void launchShell();
private:
    QTextEdit *outputEdit;
    CommandLineEdit *inputEdit;
    QPushButton *runButton;
    QPushButton *shellButton;
    QProcess *process;
};

#endif // MAINWINDOW_H
