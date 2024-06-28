#pragma once

#include "logger.h"
#include "text_highlight_plugin.h"

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
        m_stringHighlightData.clear();
        clearMovingRanges();

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
            m_rootAction->setText(i18n("Text Highlight"));

            {
                // checkbox, Highlight All Matches
                m_highlightAllMatches = actionCollection()->add<QAction>(QStringLiteral("text_highlight_highlight_all_matches"));
                m_highlightAllMatches->setText(i18n("Highlight All Matches"));
                m_highlightAllMatches->setCheckable(true);
                m_highlightAllMatches->setChecked(true);

                // checkbox, Case Sensitive Match
                m_caseSensitive = actionCollection()->add<QAction>(QStringLiteral("text_highlight_case_sensitive"));
                m_caseSensitive->setText(i18n("Case Sensitive Match"));
                m_caseSensitive->setCheckable(true);
                m_caseSensitive->setChecked(false);

                rootMenu->addAction(m_highlightAllMatches);
                rootMenu->addAction(m_caseSensitive);

                connect(m_highlightAllMatches, &QAction::changed, this, [this]() {
                    m_caseSensitive->setVisible(m_highlightAllMatches->isChecked());
                });
            }
            rootMenu->addSeparator();

            {
                auto *colorMenu = new QMenu;
                m_markColor = actionCollection()->add<QAction>(QStringLiteral("text_highlight_color_menu"));
                m_markColor->setText(i18n("Mark With ..."));
                m_markColor->setMenu(colorMenu);

                {
                    // add clear action
                    auto *clear_action = actionCollection()->add<QAction>(QStringLiteral("text_highlight_clear"));
                    clear_action->setText(i18n("Clear Selected Highlight"));
                    rootMenu->addAction(clear_action);

                    connect(clear_action, &QAction::triggered, this, [this]() {
                        auto selectionText = m_activeView->selectionText();
                        auto *doc = m_activeView->document();
                        if (m_movingRanges.find(doc) != m_movingRanges.end() && //
                            m_movingRanges[doc].find(selectionText) != m_movingRanges[doc].end()) {
                            m_movingRanges[doc].erase(selectionText);
                        }
                        m_stringHighlightData.erase(selectionText);
                    });
                }

                for (const auto &color : COLOR_SET) {
                    auto name = QString::fromUtf8(color);
                    auto *action = actionCollection()->add<QAction>(name);

                    action->setIcon(createColorIcon(color, 16));
                    action->setIconText(name);
                    colorMenu->addAction(action);
                    connect(action, &QAction::triggered, this, &TextHighlightPluginView::highlight);
                }
                rootMenu->addAction(m_markColor);
            }
        }

        connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &TextHighlightPluginView::onViewChanged);
        onViewChanged(m_mainWindow->activeView());

        m_mainWindow->guiFactory()->addClient(this);
        Logger::Log(Logger::INFO, QStringLiteral("Text Highlight activated!"), m_mainWindow);
    }

    void onViewChanged(KTextEditor::View *view);
    void onVerticalScrollPositionChanged();

    QIcon createColorIcon(const QColor &color, int size);

    void highlight(bool /*unused*/);
    void highlightAllMatches(KTextEditor::Range range = KTextEditor::Range::invalid());
    void highlightMatch(const QString &str, KTextEditor::Range range, QColor color);
    //
    void clearMovingRanges();
    //
    QObject *m_plugin{};
    KTextEditor::MainWindow *m_mainWindow{};
    QPointer<QAction> m_rootAction;
    QPointer<QAction> m_caseSensitive;
    QPointer<QAction> m_highlightAllMatches;
    QPointer<QAction> m_markColor;
    //
    QPointer<KTextEditor::View> m_activeView;
    //
    struct HighlightData {
        explicit HighlightData(QColor c, bool ham, bool cs)
            : color(c)
            , highlightAllMatches(ham)
            , caseSensitive(cs)
        {
        }
        QColor color;
        bool highlightAllMatches{};
        bool caseSensitive{};
    };
    std::unordered_map<QString, HighlightData> m_stringHighlightData;
    std::unordered_map<KTextEditor::Document *, std::unordered_map<QString, std::vector<std::unique_ptr<KTextEditor::MovingRange>>>> m_movingRanges;
};
