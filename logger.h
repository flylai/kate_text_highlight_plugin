#pragma once

#include <ktexteditor/mainwindow.h>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qstringliteral.h>
#include <QVariantMap>

class Logger
{
public:
    enum Type {
        DEBUG,
        INFO,
        WARN,
        ERROR,
    };

    static void Log(Type type, const QString &msg, KTextEditor::MainWindow *mainWindow)
    {
        QString typeStr;
        switch (type) {
        case DEBUG:
            typeStr = QStringLiteral("Debug");
            break;
        case INFO:
            typeStr = QStringLiteral("Info");
            break;
        case WARN:
            typeStr = QStringLiteral("Warning");
            break;
        case ERROR:
            typeStr = QStringLiteral("Error");
            break;
        }
        QVariantMap genericMessage;
        genericMessage.insert(QStringLiteral("category"), QStringLiteral("Text Highlight"));
        genericMessage.insert(QStringLiteral("text"), msg);
        genericMessage.insert(QStringLiteral("type"), typeStr);
        genericMessage.insert(QStringLiteral("type"), QStringLiteral("yourmessagetoken"));

        mainWindow->showMessage(genericMessage);
    }
};
