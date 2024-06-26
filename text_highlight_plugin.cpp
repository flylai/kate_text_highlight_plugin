#include <KLocalizedString>
#include <KPluginFactory>
#include <QLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QTableWidgetItem>
#include "text_highlight_plugin.h"
#include "text_highlight_plugin_view.h"

K_PLUGIN_FACTORY_WITH_JSON(TextHighlightPluginFactory, "text_highlight_plugin.json",
                           registerPlugin<TextHighlightPlugin>();)

QObject* TextHighlightPlugin::createView(KTextEditor::MainWindow *mainWindow){
    return TextHighlightPluginView::createView(this, mainWindow);
}


#include "text_highlight_plugin.moc"
