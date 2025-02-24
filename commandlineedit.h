#ifndef COMMANDLINEEDIT_H
#define COMMANDLINEEDIT_H

#include <QLineEdit>
#include <QVector>

class CommandLineEdit : public QLineEdit {
    Q_OBJECT
public:
    CommandLineEdit(QWidget *parent = nullptr);
    void addToHistory(const QString &cmd);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QVector<QString> history;
    int historyIndex;
};

#endif // COMMANDLINEEDIT_H
