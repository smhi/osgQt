//friendlyDialog.cpp
#include "friendlyDialog.h"
#include "qtTreeFilterProxyModel.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
//#include <QToolTip>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>



friendlyDialog::friendlyDialog(QWidget* parent, ModelManager* mm)
: m_parent(parent)
, m_mm(mm)
{

setWindowTitle(tr("Select Models"));

//modelList
QLabel* modelListLabel = new QLabel(tr("Models"), this);
//QLabel* modelListLabel = TitleLabel(tr("Models"), this);
modelList = new QTreeView(this);
modelList->setHeaderHidden(true);
modelList->setSelectionMode(QAbstractItemView::SingleSelection);
modelList->setSelectionBehavior(QAbstractItemView::SelectRows);
modelItems = new QStandardItemModel(this);
modelFilter = new TreeFilterProxyModel(this);
modelFilter->setDynamicSortFilter(true);
modelFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
modelFilter->setSourceModel(modelItems);
modelList->setModel(modelFilter);
connect(modelList, SIGNAL(clicked(const QModelIndex&)), SLOT(modelListClicked(const QModelIndex&)));
modelFilterEdit = new QLineEdit("", this);
modelFilterEdit->setPlaceholderText(tr("Type to filter model names"));
connect(modelFilterEdit, SIGNAL(textChanged(const QString&)), SLOT(filterModels(const QString&)));
//modelList = new QListWidget(this);
//connect(modelList, &QListWidget::itemClicked, this, &modelListClicked);

//reftimeList
QLabel* reftimeLabel = new QLabel(tr("Referencetime"), this);
//QLabel* reftimeLabel = TitleLabel(tr("Referencetime"), this);
reftimeList = new QListWidget(this);
reftimeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
connect(reftimeList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(reftimeListClicked(QListWidgetItem*)));
//connect(reftimeList, &QListWidget::itemClicked, this, &reftimeListClicked);

//selectedModelsList
QLabel* selectedModelsLabel = new QLabel(tr("Selected models"), this);
//QLabel* selectedModelsLabel = TitleLabel(tr("Selected models"), this);
selectedModelsList = new QListWidget(this);

//push button to deselect
QPushButton* deselectButton = new QPushButton(tr("Deselect"), this);
//QPushButton* deselectButton = NormalPushButton(tr("Deselect"), this);
connect(deselectButton, SIGNAL(clicked()), this, SLOT(deselectClicked()));
//connect(deselectButton, &QPushButton::clicked, this, &friendlyDialog::deselectClicked);
QPushButton* deselectAllButton = new QPushButton(tr("Deselect all"), this);
//QPushButton* deselectAllButton = NormalPushButton(tr("Deselect all"), this);
connect(deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAll()));
//connect(deselectAllButton, &QPushButton::clicked, this, &friendlyDialog::deselectAll);

//push button to load files selected, automatically hide the dialog, or?
QPushButton* dialogHideButton = new QPushButton(tr("Hide"), this);
//QPushButton* dialogHideButton = NormalPushButton(tr("Hide"), this);
connect(dialogHideButton, SIGNAL(clicked()), this, SLOT(dialogHide()));
//connect(dialogHideButton, &QPushButton::clicked, this, &friendlyDialog::dialogHide);
QPushButton* dialogApplyHideButton = new QPushButton(tr("Apply + Hide"), this);
//QPushButton* dialogApplyHideButton = NormalPushButton(tr("Apply + Hide"), this);
connect(dialogApplyHideButton, SIGNAL(clicked()), this, SLOT(applyHideClicked()));
//connect(dialogApplyHideButton, &QPushButton::clicked, this, &friendlyDialog::applyHideClicked);
QPushButton* dialogApplyButton = new QPushButton(tr("Apply"), this);
//QPushButton* dialogApplyButton = NormalPushButton(tr("Apply"), this);
connect(dialogApplyButton, SIGNAL(clicked()), this, SLOT(applyClicked()));
//connect(dialogApplyButton, &QPushButton::clicked, this, &friendlyDialog::applyClicked);

//place all widgets in layout
QHBoxLayout* hLayout1 = new QHBoxLayout();
hLayout1->addWidget(modelListLabel);
hLayout1->addWidget(modelFilterEdit);

QHBoxLayout* hLayout2 = new QHBoxLayout();
//hLayout1->addWidget(helpButton);
hLayout2->addWidget(deselectButton);
hLayout2->addWidget(deselectAllButton);

QHBoxLayout* hLayout3 = new QHBoxLayout();
hLayout3->addWidget(dialogHideButton);
hLayout3->addWidget(dialogApplyHideButton);
hLayout3->addWidget(dialogApplyButton);

QVBoxLayout* vLayout = new QVBoxLayout(this);
vLayout->addLayout(hLayout1);
vLayout->addWidget(modelList);
vLayout->addWidget(reftimeLabel);
vLayout->addWidget(reftimeList);
vLayout->addWidget(selectedModelsLabel);
vLayout->addWidget(selectedModelsList);
vLayout->addLayout(hLayout2);
vLayout->addLayout(hLayout3);
}


void friendlyDialog::updateModelList()
{
m_modelGroups = m_mm->getFieldModelGroups();

}


void friendlyDialog::getModel()
{

}

//maybe just maybe unnecessary
void friendlyDialog::closeFriendlyDialogEvent(QCloseEvent* e)
{
  Q_EMIT dialogHide();
}

/*
void dialogHide()
{
  hide();
}
*/
QString friendlyDialog::getSelectedModelString()
{

}

void friendlyDialog::deselectAll()
{

}

static QString currentItem(QListWidget* list)
{

}

void friendlyDialog::filterModels(const QString& filtertext)
{
  modelFilter->setFilterFixedString(filtertext);
   if (!filtertext.isEmpty()) {
      modelList->expandAll();
      if (modelFilter->rowCount() > 0) modelList->scrollTo(modelFilter->index(0, 0));
   }
}

void friendlyDialog::modelListClicked(const QModelIndex& filterIndex)
{
   const QModelIndex index = modelFilter->mapToSource(filterIndex);
   QStandardItem* clickedItem = modelItems->itemFromIndex(index);
   QStandardItem* parentItem = clickedItem->parent();
   if (!parentItem) {
     return;
   }
   
   reftimeList->clear();
   //selectedModelsList->clear();

   const int indexM = clickedItem->row();
   const int indexMGR = parentItem->data().toInt();

   currentModel = m_modelGroups[indexMGR].models[indexM].modelName;
   //const std::set<std::string> refTimes = ->getReferenceTimes(currentModel->text().toStdString());
}

void friendlyDialog::updateReferencetime()
{

}

void friendlyDialog::reftimeListClicked(QListWidgetItem*)
{

}


void friendlyDialog::deselectClicked()
{

}


void friendlyDialog::applyHideClicked()
{

}

void friendlyDialog::applyClicked()
{

}
