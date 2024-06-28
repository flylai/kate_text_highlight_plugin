#include "text_highlight_plugin_view.h"
#include "logger.h"

QObject *TextHighlightPluginView::createView(TextHighlightPlugin *plugin, KTextEditor::MainWindow *mainWindow)
{
    return new TextHighlightPluginView(plugin, mainWindow);
}

void TextHighlightPluginView::onViewChanged(KTextEditor::View *view)
{
    auto oldView = m_activeView;

    if (oldView == view) {
        return;
    }
    m_activeView = view;

    if (view && view->focusProxy()) {
        view->focusProxy()->installEventFilter(this);
        connect(view, &KTextEditor::View::verticalScrollPositionChanged, this, &TextHighlightPluginView::onVerticalScrollPositionChanged);
        //
        // https://api.kde.org/frameworks/ktexteditor/html/classKTextEditor_1_1MovingRange.html
        // Chapter `MovingRange Example`
        //
        connect(view->document(),
                &KTextEditor::Document::aboutToInvalidateMovingInterfaceContent,
                this,
                &TextHighlightPluginView::clearMovingRanges,
                Qt::UniqueConnection);
        connect(view->document(),
                &KTextEditor::Document::aboutToDeleteMovingInterfaceContent,
                this,
                &TextHighlightPluginView::clearMovingRanges,
                Qt::UniqueConnection);
        onVerticalScrollPositionChanged();
    }

    if (oldView && oldView->focusProxy()) {
        oldView->focusProxy()->removeEventFilter(this);
        disconnect(oldView, &KTextEditor::View::verticalScrollPositionChanged, this, &TextHighlightPluginView::onVerticalScrollPositionChanged);
    }
}

void TextHighlightPluginView::onVerticalScrollPositionChanged()
{
    if (m_activeView && !m_stringHighlightData.empty()) {
        for (const auto &[str, data] : m_stringHighlightData) {
            if (!data.highlightAllMatches) {
                continue;
            }
            highlightAllMatches();
        }
    }
}

QIcon TextHighlightPluginView::createColorIcon(const QColor &color, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.drawRect(0, 0, size, size);
    return {pixmap};
}

void TextHighlightPluginView::highlight(bool /*unused*/)
{
    auto *action = qobject_cast<QAction *>(sender());
    auto color = QColor(action->iconText());
    auto selectionText = m_mainWindow->activeView()->selectionText();
    m_stringHighlightData.insert({selectionText, HighlightData(color, m_highlightAllMatches->isChecked(), m_caseSensitive->isChecked())});

    if (m_highlightAllMatches->isChecked()) {
        highlightAllMatches();
    } else {
        highlightMatch(selectionText, m_mainWindow->activeView()->selectionRange(), color);
    }

    Logger::Log(Logger::INFO, selectionText, m_mainWindow);
}

void TextHighlightPluginView::highlightAllMatches(KTextEditor::Range range)
{
    if (!m_mainWindow || !m_mainWindow->activeView()) {
        return;
    }
    auto lineRange = range.toLineRange();
    auto *doc = m_activeView->document();
    const int startLine = lineRange.isValid() ? lineRange.start() : m_activeView->firstDisplayedLine();
    const int endLine = lineRange.isValid() ? lineRange.end() : m_activeView->lastDisplayedLine();

    auto &strs = m_movingRanges[doc];
    if (range.isValid()) {
        for (auto &[_, v] : strs) {
            v.erase(std::remove_if(v.begin(),
                                   v.end(),
                                   [lineRange](const std::unique_ptr<KTextEditor::MovingRange> &r) {
                                       return lineRange.overlapsLine(r->start().line());
                                   }),
                    v.end());
        }
    } else {
        for (auto &[_, v] : strs) {
            v.clear();
        }
    }

    for (int line = startLine; line < endLine; line++) {
        for (const auto &[str, highlightData] : m_stringHighlightData) {
            QString content = doc->line(line);
            for (qsizetype i = 0;;) {
                i = content.indexOf(str, i, highlightData.caseSensitive ? Qt::CaseInsensitive : Qt::CaseInsensitive);
                if (i == -1) {
                    // Not found
                    break;
                }
                highlightMatch(str, KTextEditor::Range(line, i, line, i + str.size()), highlightData.color);
                i += str.size();
            }
        }
    }
}

void TextHighlightPluginView::highlightMatch(const QString &str, KTextEditor::Range range, QColor color)
{
    KTextEditor::MovingRange *movingRange = m_activeView->document()->newMovingRange(range);
    const KTextEditor::Attribute::Ptr attr([color] {
        auto *attr = new KTextEditor::Attribute;
        QBrush brush(color, Qt::BrushStyle::SolidPattern);
        attr->setBackground(brush);
        return attr;
    }());
    movingRange->setAttribute(attr);
    m_movingRanges[m_activeView->document()][str].emplace_back(movingRange);
}

void TextHighlightPluginView::clearMovingRanges()
{
    m_movingRanges.clear();
}
