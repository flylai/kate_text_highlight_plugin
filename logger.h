#pragma once

#include "ktexteditor_utils.h"
#include <ktexteditor/mainwindow.h>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qstringliteral.h>

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
        genericMessage.insert(QStringLiteral("type"), QStringLiteral("Warning"));

        Utils::showMessage(genericMessage, mainWindow);
    }
};
