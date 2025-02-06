#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QProcess>

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
    QLineEdit *inputEdit;
    QPushButton *runButton;
    QPushButton *shellButton;
    QProcess *process;
};

#endif // MAINWINDOW_H
