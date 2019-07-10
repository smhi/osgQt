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
#include <iostream>

#include "diUtilities.h"
#include "util/misc_util.h"

#include <puTools/miStringFunctions.h>

/* completely useless right now
namespace { //anonymous
const int ROLE_MODELGROUP = Qt::UserRole +1;
}//end anon namespace
*/

friendlyDialog::friendlyDialog(QWidget* parent, ModelManager* mm)
: m_parent(parent)
, m_mm(mm)
{

setWindowTitle(tr("Select Models"));

//archive bool
useArchive = false;

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
selectedModelsList->setSelectionMode(QAbstractItemView::SingleSelection);
connect(selectedModelsList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(selectedModelsListClicked(QListWidgetItem*)));

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
connect(dialogHideButton, SIGNAL(clicked()), this, SLOT(dHide()));
//connect(dialogHideButton, SIGNAL(clicked()), this, SLOT(dialogHide()));
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
updateDialog();

}


void friendlyDialog::updateModelList()
{

   modelItems->clear();
   modelFilterEdit->clear();

   int nr_m = m_modelGroups.size();
   if (nr_m == 0) return;

   if (useArchive) {
    for (int l = 0; l < nr_m; l++) {
       if(m_modelGroups[l].groupType == FieldModelGroupInfo::ARCHIVE_GROUP) {
       addModelGroup(l);
        }
      }
    }

   for (int i = 0; i < nr_m; i++) {
    if (m_modelGroups[i].groupType == FieldModelGroupInfo::STANDARD_GROUP) {
     addModelGroup(i);
        }
    }
   if (selectedModels.size() > 0) deselectAll();
}

void friendlyDialog::updateDialog()
{
   m_modelGroups = m_mm->getFieldModelGroups();
   updateModelList();
}

void friendlyDialog::archiveMode(bool on)
{
   useArchive = on;
   updateModelList();
}

void friendlyDialog::getModel()
{

}

//maybe just maybe unnecessary
void friendlyDialog::closeFriendlyDialogEvent(QCloseEvent* e)
{
  Q_EMIT dialogHide();
}

/*QString friendlyDialog::getSelectedModelString()
{
   QString qstr;
//   if (modelList->current)
}*/

void friendlyDialog::deselectAll()
{
   int n = reftimeList->count();
   if (n > 0) {
       reftimeList->blockSignals(true);
       reftimeList->clearSelection();
       reftimeList->blockSignals(false);
     }
    selectedModelsList->clear(); //yeet
}

static QString currentItem(QListWidget* list)
{

}

void friendlyDialog::addModelGroup(int modelGroupIndex)
{
   const FieldModelGroupInfo& mgr = m_modelGroups[modelGroupIndex];
   QStandardItem* group = new QStandardItem(QString::fromStdString(mgr.groupName));
   group->setData(modelGroupIndex/*, ROLE_MODELGROUP*/);
   group->setFlags(Qt::ItemIsEnabled);
   for (const FieldModelInfo& fdmi : mgr.models) {
   QStandardItem* child = new QStandardItem(QString::fromStdString(fdmi.modelName));
   //child->setToolTip(QString::fromStdString(fdmi.setupInfo).split("", QString::SkipEmptyParts)-join("\n"));
   child->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
   group->appendRow(child);
      }
   modelItems->appendRow(group);
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
   selectedModelsList->clear();
   selectedModels.clear();

   const int indexM = clickedItem->row();
   const int indexMGR = parentItem->data().toInt();

   currentModel = m_modelGroups[indexMGR].models[indexM].modelName;
   refTimes = m_mm->getReferenceTimes(currentModel);
   fileNames = m_mm->getFileNames(currentModel);
   reftime_filename.clear();
   // Here we must assume that there are equal number of reftimes and filenames
   if (refTimes.size() == fileNames.size())
   {
     // put in map.
     size_t i = 0;
     std::set<std::string>::iterator it = refTimes.begin();
     for (; it!=refTimes.end(); ++it) {
       reftime_filename[*it] = fileNames[i];
       i++;
     }
     
   }
   //const std::set<std::string> refTimes = m_mm->getReferenceTimes(currentModel->text().toStdString());
   if (refTimes.empty()) {
     const QString currentQStringModel = QString::fromStdString(currentModel);
    if (!selectedModelsList->count() || currentQStringModel != currentItem(selectedModelsList)) {
      selectedModelsList->addItem(currentQStringModel);
    } else {
      return;
     }
   } else {
   for (const std::string& rt : refTimes) reftimeList->addItem(QString::fromStdString(rt));
   reftimeList->setCurrentRow(reftimeList->count() - 1);
   //updateReferencetime();
   }
}

void friendlyDialog::updateReferencetime()
{
   //reftimeList->clear();
   
}

void friendlyDialog::reftimeListClicked(QListWidgetItem* item)
{
   if (!reftimeList->count()) return;
    if (item->isSelected()) {
      SelectedModel SM;
      SM.modelName = currentModel;
      SM.refTime = reftimeList->currentItem()->text().toStdString();
      selectedModels.push_back(SM); 
      std::string text = SM.modelName + " " + SM.refTime;
      int newCurrent = selectedModelsList->count();
      selectedModelsList->addItem(QString::fromStdString(text));
      selectedModelsList->setCurrentRow(newCurrent);
      selectedModelsList->item(newCurrent)->setSelected(true);

    } else if (!item->isSelected()) {
      std::string refTime = item->text().toStdString();
      int n = selectedModels.size();
      int u;
      for (u = 0; u < n; u++) {
            if (selectedModels[u].modelName == currentModel && selectedModels[u].refTime == refTime)   {
             selectedModelsList->takeItem(u);
             break;
          }
        }
   if (u < n) {
     for (int o = u; o < n -1; o++)
          selectedModels[o] = selectedModels[o + 1];
         selectedModels.pop_back();
       }
   if ( selectedModelsList->count() ) {
       int newCurrent = selectedModelsList->count() - 1;
       if ( u < selectedModelsList->count()) newCurrent = u;
       selectedModelsList->setCurrentRow(newCurrent);
       selectedModelsList->item(newCurrent)->setSelected(true);
       
       } 
   }
}
/*
void friendlyDialog::reftimeListClicked(QListWidgetItem*)
{

}*/

void friendlyDialog::selectedModelsListClicked(QListWidgetItem * item)
{
   int index = selectedModelsList->row(item);
   if (index < 0 || selectedModels.size() == 0) return;
}

void friendlyDialog::deselectClicked()
{
   if (selectedModelsList->count()) {
      int row = selectedModelsList->currentRow();
      selectedModelsList->takeItem(row);
   }
}


void friendlyDialog::applyHideClicked()
{
   dHide();
   if (!selectedModelsList->count()) return;
   // get all items from selectedModelsList
   std::set<std::string> smlitems;
   for (int i = 0; i < selectedModelsList->count(); i++)
   {
     QListWidgetItem * tmp = selectedModelsList->item(i);
     // Avoid duplicates
     smlitems.insert(tmp->text().toStdString());
   }
   // Construct the m_selectedModelFiles
   std::string tmpmodelName;
   std::string refTime;
   std::string fileName;
   ModelFileInfo_v modelfileinfos;
   // Only one member for the moment;
   SelectedModelInfo_v modelinfos;
   std::set<std::string>::iterator it = smlitems.begin();
   for (; it!=smlitems.end(); ++it) {
     std::cerr << *it << std::endl;
     std::vector<std::string> tokens = miutil::split_protected(*it,' ', ' ', " ", true);
     if (tokens.size() == 2) {
       tmpmodelName = tokens[0];
       refTime = tokens[1];
       // Get filename from reftime;
       if (reftime_filename.count(refTime))
       {
         fileName = reftime_filename[refTime];
       }
       ModelFileInfo mfi(fileName, refTime);
       modelfileinfos.push_back(mfi);
       std::cerr << tmpmodelName << ", " << refTime << ", " << fileName << std::endl;
     }
   }
   std::cerr << std::endl;
   SelectedModelInfo smi(tmpmodelName,  modelfileinfos);
   modelinfos.push_back(smi);
   m_selectedModelFiles = modelinfos;
   emit dialogApply();
}

void friendlyDialog::applyClicked()
{
  if (!selectedModelsList->count()) return;
  // get all items from selectedModelsList
   std::set<std::string> smlitems;
   for (int i = 0; i < selectedModelsList->count(); i++)
   {
     QListWidgetItem * tmp = selectedModelsList->item(i);
     // Avoid duplicates
     smlitems.insert(tmp->text().toStdString());
   }
   // Construct the m_selectedModelFiles
   std::string tmpmodelName;
   std::string refTime;
   std::string fileName;
   ModelFileInfo_v modelfileinfos;
   // Only one member for the moment;
   SelectedModelInfo_v modelinfos;
   std::set<std::string>::iterator it = smlitems.begin();
   for (; it!=smlitems.end(); it++) {
     std::cerr << *it << std::endl;
     std::vector<std::string> tokens = miutil::split_protected(*it,' ', ' ', " ", true);
     if (tokens.size() >= 2) {
       tmpmodelName = tokens[0];
       refTime = tokens[1];
       // Get filename from reftime;
       if (reftime_filename.count(refTime))
       {
         fileName = reftime_filename[refTime];
       }
       ModelFileInfo mfi(fileName, refTime);
       modelfileinfos.push_back(mfi);
       std::cerr << tmpmodelName << ", " << refTime << ", " << fileName << std::endl;
     }
   }
   std::cerr << std::endl;
   SelectedModelInfo smi(tmpmodelName,  modelfileinfos);
   modelinfos.push_back(smi);
   m_selectedModelFiles = modelinfos;
  emit dialogApply();
}

void friendlyDialog::dHide()
{
   this->hide(); //yeet
}
