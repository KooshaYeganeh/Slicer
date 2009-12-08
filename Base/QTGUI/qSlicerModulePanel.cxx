#include "qSlicerModulePanel.h"

// SlicerQT includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"

// qCTK includes
#include <qCTKCollapsibleButton.h>
#include <qCTKFittedTextBrowser.h>

// QT includes
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QTextBrowser>
#include <QWebView>

//---------------------------------------------------------------------------
struct qSlicerModulePanelPrivate: public qCTKPrivate<qSlicerModulePanel>
{
  void setupUi(QWidget * widget);

  QTextBrowser*          HelpLabel;
  //QWebView*              HelpLabel;
  QBoxLayout*            Layout;
  QScrollArea*           ScrollArea;
  qCTKCollapsibleButton* HelpCollapsibleButton;
};

//---------------------------------------------------------------------------
qSlicerModulePanel::qSlicerModulePanel(QWidget* parent, Qt::WindowFlags f)
  :qSlicerAbstractModulePanel(parent, f)
{
  QCTK_INIT_PRIVATE(qSlicerModulePanel);
  QCTK_D(qSlicerModulePanel);
  d->setupUi(this);
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::setModule(const QString& moduleName)
{
  QCTK_D(qSlicerModulePanel);

  qSlicerAbstractModule * module = 0;
  
  if (!moduleName.isEmpty())
    {
    module = qSlicerApplication::application()->moduleManager()->getModule(moduleName);
    Q_ASSERT(module);
    }
  
  // Retrieve current module associated with the module panel
  QLayoutItem* item = d->Layout->itemAt(1);
  qSlicerAbstractModuleWidget* currentModuleWidget = 
    item ? qobject_cast<qSlicerAbstractModuleWidget*>(item->widget()) : 0;

  // If module is already set, return.
  if (module && (module->widgetRepresentation() == currentModuleWidget))
    {
    return;
    }

  if (currentModuleWidget)
    {
    // Remove the current module
    this->removeModule(currentModuleWidget->name());
    }

  if (module)
    {
    qDebug() << "Show module (name):" << moduleName;
    // Add the new module
    this->addModule(module->name());
    }
  else
    {
    //d->HelpLabel->setHtml("");
    }
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::addModule(const QString& moduleName)
{
  qSlicerAbstractModule * module =
    qSlicerApplication::application()->moduleManager()->getModule(moduleName); 
  Q_ASSERT(module);
  
  qSlicerAbstractModuleWidget * moduleWidget = module->widgetRepresentation();
  Q_ASSERT(moduleWidget);
  
  QCTK_D(qSlicerModulePanel);
  
  // Update module layout
  moduleWidget->layout()->setContentsMargins(0, 0, 0, 0);

  // Insert module in the panel
  d->Layout->insertWidget(1, moduleWidget);

  moduleWidget->setSizePolicy(QSizePolicy::Ignored, moduleWidget->sizePolicy().verticalPolicy());
  moduleWidget->setVisible(true);

  QString help = module->helpText();
  d->HelpCollapsibleButton->setVisible(!help.isEmpty());
  d->HelpLabel->setHtml(help);
  //d->HelpLabel->load(QString("http://www.slicer.org/slicerWiki/index.php?title=Modules:Transforms-Documentation-3.4&useskin=chick"));

  emit moduleAdded(module->name());
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::removeModule(const QString& moduleName)
{
  qSlicerAbstractModule * module =
    qSlicerApplication::application()->moduleManager()->getModule(moduleName); 
  Q_ASSERT(module);
  
  qSlicerAbstractModuleWidget * moduleWidget = module->widgetRepresentation();
  Q_ASSERT(moduleWidget);
  
  QCTK_D(qSlicerModulePanel);

  int index = d->Layout->indexOf(moduleWidget);
  if (index == -1)
    {
    return;
    }

  //emit moduleAboutToBeRemoved(moduleWidget->module());

  // Remove widget from layout
  //d->Layout->removeWidget(module);
  d->Layout->takeAt(index);

  moduleWidget->setVisible(false);
  moduleWidget->setParent(0);

  // if nobody took ownership of the module, make sure it both lost its parent and is hidden
//   if (moduleWidget->parent() == d->Layout->parentWidget())
//     {
//     moduleWidget->setVisible(false);
//     moduleWidget->setParent(0);
//     }

  emit moduleRemoved(module->name());
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::removeAllModule()
{
  this->setModule("");
}

//---------------------------------------------------------------------------
// qSlicerModulePanelPrivate methods

//---------------------------------------------------------------------------
void qSlicerModulePanelPrivate::setupUi(QWidget * widget)
{
  QWidget* panel = new QWidget;
  this->HelpCollapsibleButton = new qCTKCollapsibleButton("Help");
  this->HelpCollapsibleButton->setCollapsed(true);
  this->HelpCollapsibleButton->setSizePolicy(
    QSizePolicy::Ignored, QSizePolicy::Minimum);
    
  // Note: QWebView could be used as an alternative to qCTKFittedTextBrowser ?
  //this->HelpLabel = new QWebView;
  this->HelpLabel = static_cast<QTextBrowser*>(new qCTKFittedTextBrowser);
  this->HelpLabel->setOpenExternalLinks(true);
  this->HelpLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->HelpLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->HelpLabel->setFrameShape(QFrame::NoFrame);
  
  QPalette p = this->HelpLabel->palette();
  p.setBrush(QPalette::Window, QBrush ());
  this->HelpLabel->setPalette(p);

  QGridLayout* helpLayout = new QGridLayout(this->HelpCollapsibleButton);
  helpLayout->addWidget(this->HelpLabel);

  this->Layout = new QVBoxLayout(panel);
  this->Layout->addWidget(this->HelpCollapsibleButton);
  this->Layout->addItem(
    new QSpacerItem(0, 0, QSizePolicy::Minimum, 
                    QSizePolicy::MinimumExpanding));

  this->ScrollArea = new QScrollArea;
  this->ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->ScrollArea->setWidget(panel);
  this->ScrollArea->setWidgetResizable(true);
  
  QGridLayout* gridLayout = new QGridLayout;
  gridLayout->addWidget(this->ScrollArea);
  gridLayout->setContentsMargins(0,0,0,0);
  widget->setLayout(gridLayout);
}
