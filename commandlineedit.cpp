#include "commandlineedit.h"
#include <QKeyEvent>

CommandLineEdit::CommandLineEdit(QWidget *parent)
    : QLineEdit(parent), historyIndex(-1)
{
    setPlaceholderText("Enter command...");
}

void CommandLineEdit::addToHistory(const QString &cmd)
{
    if (!cmd.isEmpty() && (history.isEmpty() || history.last() != cmd))
    {
        history.append(cmd);
    }
    historyIndex = history.size();
}

void CommandLineEdit::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Up:
        if (!history.isEmpty() && historyIndex > 0)
        {
            historyIndex--;
            setText(history[historyIndex]);
        }
        break;
    case Qt::Key_Down:
        if (historyIndex < history.size() - 1)
        {
            historyIndex++;
            setText(history[historyIndex]);
        }
        else
        {
            historyIndex = history.size();
            clear();
        }
        break;
    default:
        QLineEdit::keyPressEvent(event);
    }
}
