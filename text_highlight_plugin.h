#pragma once

#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QSettings>
// forward declaration

class TextHighlightPlugin : public KTextEditor::Plugin {
  Q_OBJECT
public:
  explicit TextHighlightPlugin(QObject *parent,
                         const QList<QVariant> & = QList<QVariant>())
      : KTextEditor::Plugin(parent) {}

  QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};

