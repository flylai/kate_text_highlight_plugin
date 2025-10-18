#include "text_highlight_plugin_view.h"
#include <QChar>
#include <QObject>
#include <QStringLiteral>

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
        // https://api.kde.org/ktexteditor-movingrange.html#movingrange-example
        // Chapter `MovingRange Example`
        //
        connect(view->document(),
                &KTextEditor::Document::aboutToInvalidateMovingInterfaceContent,
                this,
                &TextHighlightPluginView::clearMovingRanges,
                Qt::UniqueConnection);
#if KTEXTEDITOR_VERSION < QT_VERSION_CHECK(6, 9, 0)
        connect(view->document(),
                &KTextEditor::Document::aboutToDeleteMovingInterfaceContent,
                this,
                &TextHighlightPluginView::clearMovingRanges,
                Qt::UniqueConnection);
#endif // KTEXTEDITOR_VERSION < QT_VERSION_CHECK(6, 9, 0)
        connect(view->document(), &KTextEditor::Document::textChanged, this, &TextHighlightPluginView::onTextChanged);
        connect(view->document(), &KTextEditor::Document::aboutToClose, this, &TextHighlightPluginView::onDocumentClosed);
        //
        onVerticalScrollPositionChanged();
    }

    if (oldView && oldView->focusProxy()) {
        oldView->focusProxy()->removeEventFilter(this);
        disconnect(oldView, &KTextEditor::View::verticalScrollPositionChanged, this, &TextHighlightPluginView::onVerticalScrollPositionChanged);
    }
}

void TextHighlightPluginView::onVerticalScrollPositionChanged()
{
    highlightCurrentViewport();
}

void TextHighlightPluginView::onTextChanged(KTextEditor::Document * /*unused*/)
{
    highlightCurrentViewport();
}

void TextHighlightPluginView::onDocumentClosed(KTextEditor::Document *doc)
{
    m_documentHighlightData.erase(doc);
    m_documentMovingRanges.erase(doc);
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

void TextHighlightPluginView::onApplyHighlightColor(bool /*unused*/)
{
    auto selectionText = m_activeView->selectionText();
    if (selectionText.isEmpty() || selectionText.contains(QLatin1Char('\n'))) {
        return;
    }

    auto *currentDocument = m_activeView->document();
    auto *action = qobject_cast<QAction *>(sender());

    auto &currentDocumentHighlightData = m_documentHighlightData[currentDocument];
    auto &currentDocumentMovingRanges = m_documentMovingRanges[currentDocument];

    const auto colorStr = action->iconText();
    if (colorStr == QStringLiteral("Clear")) {
        currentDocumentHighlightData.erase(selectionText);
        currentDocumentMovingRanges.erase(selectionText);
        return;
    }

    auto color = QColor(colorStr);
    currentDocumentHighlightData[selectionText] = color;
    currentDocumentMovingRanges[selectionText].clear();

    highlightCurrentViewport();
}

void TextHighlightPluginView::highlightCurrentViewport(KTextEditor::Range range)
{
    if (!m_mainWindow || !m_mainWindow->activeView()) {
        return;
    }
    auto lineRange = range.toLineRange();
    const int startLine = lineRange.isValid() ? lineRange.start() : m_activeView->firstDisplayedLine();
    const int endLine = lineRange.isValid() ? lineRange.end() : m_activeView->lastDisplayedLine();

    m_documentMovingRanges[m_activeView->document()].clear();

    for (int line = startLine; line < endLine; line++) {
        highlightLine(line);
    }
}

void TextHighlightPluginView::highlightLine(int line)
{
    if (m_activeView == nullptr) {
        return;
    }
    QString content = m_activeView->document()->line(line);

    auto findAndHighlight = [this, line, &content](const QString &str, const QColor &color) {
        for (qsizetype i = 0;;) {
            i = content.indexOf(str, i);
            if (i == -1) {
                // Not found
                break;
            }
            highlightMatched(str, KTextEditor::Range(line, i, line, i + str.size()), color);
            i += str.size();
        }
    };
    if (!m_activeView || !m_documentHighlightData.contains(m_activeView->document())) {
        return;
    }
    const auto &highlightData = m_documentHighlightData[m_activeView->document()];
    for (const auto &[str, color] : highlightData) {
        findAndHighlight(str, color);
    }
}

void TextHighlightPluginView::highlightMatched(const QString &str, KTextEditor::Range range, QColor color)
{
    KTextEditor::MovingRange *movingRange = m_activeView->document()->newMovingRange(range);
    const KTextEditor::Attribute::Ptr attr([color] {
        auto *attr = new KTextEditor::Attribute;
        QBrush brush(color, Qt::BrushStyle::SolidPattern);
        attr->setBackground(brush);
        return attr;
    }());
    movingRange->setAttribute(attr);
    m_documentMovingRanges[m_activeView->document()][str].emplace_back(movingRange);
}

void TextHighlightPluginView::clearMovingRanges(KTextEditor::Document *doc)
{
    m_documentMovingRanges[doc].clear();
}
