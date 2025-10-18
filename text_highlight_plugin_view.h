#pragma once

#include "logger.h"
#include "text_highlight_plugin.h"

#include <QAction>
#include <QColor>
#include <QIcon>
#include <QKeySequence>
#include <QMap>
#include <QMenu>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QTimer>

#include <KActionCollection>
#include <KActionMenu>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/MovingRange>
#include <KTextEditor/Range>
#include <KTextEditor/View>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>
#include <ktexteditor/document.h>
#include <ktexteditor/mainwindow.h>

#include <memory>
#include <unordered_map>
#include <vector>

class TextHighlightPluginView : public QObject, public KXMLGUIClient
{
    Q_OBJECT

    static constexpr const char *COLOR_SET[] = {
        "#96CA2D",
        "#0EEAFF",
        "#EF5350",
        "#F77A52",
        "#9575CD",
        "#FFF176",
        "#607D8B",
    };

public:
    static QObject *createView(TextHighlightPlugin *plugin, KTextEditor::MainWindow *mainWindow);
    ~TextHighlightPluginView()
    {
        disconnect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &TextHighlightPluginView::onViewChanged);
        onViewChanged(nullptr);
        m_documentHighlightData.clear();
        m_documentMovingRanges.clear();

        m_mainWindow->guiFactory()->removeClient(this);
    }

private:
    TextHighlightPluginView(TextHighlightPlugin *plugin, KTextEditor::MainWindow *mainWindow)
        : QObject(mainWindow)
        , m_plugin(plugin)
        , m_mainWindow(mainWindow)
    {
        KXMLGUIClient::setComponentName(QStringLiteral("text_highlight_plugin"), i18n("Text Highlight"));
        setXMLFile(QStringLiteral("ui.rc"));
        {
            // root menu
            auto *rootMenu = new QMenu();
            m_rootAction = actionCollection()->add<QAction>(QStringLiteral("text_highlight_menu"));
            m_rootAction->setMenu(rootMenu);
            m_rootAction->setText(i18n("Mark With ..."));

            {
                // clear highlights action
                auto name = QString::fromUtf8("Clear");
                auto *action = actionCollection()->add<QAction>(name);
                action->setIcon(createColorIcon(QColorConstants::Transparent, 16));
                action->setIconText(name);
                rootMenu->addAction(action);
                connect(action, &QAction::triggered, this, &TextHighlightPluginView::onApplyHighlightColor);
            }

            for (const auto &color : COLOR_SET) {
                auto name = QString::fromUtf8(color);
                auto *action = actionCollection()->add<QAction>(name);

                action->setIcon(createColorIcon(color, 16));
                action->setIconText(name);
                rootMenu->addAction(action);
                connect(action, &QAction::triggered, this, &TextHighlightPluginView::onApplyHighlightColor);
            }
        }

        connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &TextHighlightPluginView::onViewChanged);
        onViewChanged(m_mainWindow->activeView());

        m_mainWindow->guiFactory()->addClient(this);
        Logger::Log(Logger::INFO, QStringLiteral("Text Highlight activated!"), m_mainWindow);
    }

    // slots
    void onViewChanged(KTextEditor::View *view);
    void onVerticalScrollPositionChanged();
    void onTextChanged(KTextEditor::Document *doc);
    void onDocumentClosed(KTextEditor::Document *doc);
    void onApplyHighlightColor(bool /*unused*/);
    void clearMovingRanges(KTextEditor::Document *doc);

    QIcon createColorIcon(const QColor &color, int size);

    void highlightCurrentViewport(KTextEditor::Range range = KTextEditor::Range::invalid());
    void highlightLine(int line);
    void highlightMatched(const QString &str, KTextEditor::Range range, QColor color);
    //
    QObject *m_plugin{};
    KTextEditor::MainWindow *m_mainWindow{};
    QPointer<QAction> m_rootAction;
    //
    QPointer<KTextEditor::View> m_activeView;
    QTimer m_highlightTimer;

    // document highlights
    std::unordered_map<KTextEditor::Document *, std::unordered_map<QString, QColor>> m_documentHighlightData;
    // document highlights moving ranges
    std::unordered_map<KTextEditor::Document *, std::unordered_map<QString, std::vector<std::unique_ptr<KTextEditor::MovingRange>>>> m_documentMovingRanges;
};
