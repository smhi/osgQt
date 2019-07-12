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

namespace { //anonymous
const int ROLE_MODELGROUP = Qt::UserRole +1;
}//end anon namespace


friendlyDialog::friendlyDialog(QWidget* parent, ModelManager* mm)
: m_parent(parent)
, m_mm(mm)
{

setWindowTitle(tr("Select Models"));

//archive bool
useArchive = false;

//modelList
QLabel* modelListLabel = new QLabel(tr("Models"), this);
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
modelFilterEdit->setMaxLength(64);
modelFilterEdit->setPlaceholderText(tr("Type to filter model names"));
connect(modelFilterEdit, SIGNAL(textChanged(const QString&)), SLOT(filterModels(const QString&)));

//reftimeList
QLabel* reftimeLabel = new QLabel(tr("Referencetime"), this);
reftimeList = new QListWidget(this);
reftimeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
reftimeList->setSortingEnabled(true);
connect(reftimeList, SIGNAL(itemSelectionChanged()), this, SLOT(getSelectedReftimes()));

//selectedModelsList
QLabel* selectedModelsLabel = new QLabel(tr("Selected models"), this);
selectedModelsList = new QListWidget(this);
selectedModelsList->setSelectionMode(QAbstractItemView::SingleSelection);
selectedModelsList->setSortingEnabled(true);
connect(selectedModelsList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(selectedModelsListClicked(QListWidgetItem*)));

//push button to deselect
QPushButton* deselectButton = new QPushButton(tr("Deselect"), this);
connect(deselectButton, SIGNAL(clicked()), this, SLOT(deselectClicked()));

QPushButton* deselectAllButton = new QPushButton(tr("Deselect all"), this);
connect(deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAll()));

//push button to load files selected, automatically hide the dialog, or?
QPushButton* dialogHideButton = new QPushButton(tr("Hide"), this);
connect(dialogHideButton, SIGNAL(clicked()), this, SLOT(dHide()));

QPushButton* dialogApplyHideButton = new QPushButton(tr("Apply + Hide"), this);
connect(dialogApplyHideButton, SIGNAL(clicked()), this, SLOT(applyHideClicked()));

QPushButton* dialogApplyButton = new QPushButton(tr("Apply"), this);
connect(dialogApplyButton, SIGNAL(clicked()), this, SLOT(applyClicked()));

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

//update the dialog with modelgroups...
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

void friendlyDialog::closeFriendlyDialogEvent(QCloseEvent* e)
{
  Q_EMIT dialogHide();
}

void friendlyDialog::deselectAll()
{
   int n = reftimeList->count();
   if (n > 0) {
       reftimeList->blockSignals(true);
       reftimeList->clearSelection();
       reftimeList->blockSignals(false);
     }
    selectedModelsList->clear(); 
}

void friendlyDialog::addModelGroup(int modelGroupIndex)
{
   const FieldModelGroupInfo& mgr = m_modelGroups[modelGroupIndex];
   QStandardItem* group = new QStandardItem(QString::fromStdString(mgr.groupName));
   group->setData(modelGroupIndex, ROLE_MODELGROUP);
   group->setFlags(Qt::ItemIsEnabled);
   for (const FieldModelInfo& fdmi : mgr.models) {
   QStandardItem* child = new QStandardItem(QString::fromStdString(fdmi.modelName));
   //child->setToolTip(QString::fromStdString(fdmi.setupInfo).split("", QString::SkipEmptyParts)-join("\n"));
   child->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
   group->appendRow(child);
      }
   modelItems->appendRow(group);
}


static QString currentItem(QListWidget* list)
{
  QListWidgetItem* i = list->currentItem();
  return i ? i->text() : QString();
}

void friendlyDialog::filterModels(const QString& filtertext)
{
   //std::cerr << "filterModels: " << filtertext.toStdString() << std::endl;
   modelFilter->setFilterFixedString(filtertext);
   if (!filtertext.isEmpty()) {
      modelList->expandAll();
      if (modelFilter->rowCount() > 0) modelList->scrollTo(modelFilter->index(0, 0));
   }
}

void friendlyDialog::modelListClicked(const QModelIndex& filterIndex)
{
   //std::cerr << "modelListClicked: " << filterIndex.row() << std::endl;
   const QModelIndex index = modelFilter->mapToSource(filterIndex);
   QStandardItem* clickedItem = modelItems->itemFromIndex(index);
   QStandardItem* parentItem = clickedItem->parent();
   if (!parentItem) {
     return;
   }
   reftimeList->blockSignals(true);
   reftimeList->clearSelection();
   reftimeList->clear();
   reftimeList->blockSignals(false);
   selectedModelsList->clear();
   selectedModels.clear();

   const int indexM = clickedItem->row();
   const int indexMGR = parentItem->data(ROLE_MODELGROUP).toInt();
   ////std::cerr << "indexM: " << indexM << std::endl;
   ////std::cerr << "indexMGR: " << indexMGR << std::endl;
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
   //std::cerr << "reftimeList->count: " << reftimeList->count() << std::endl;
   }
}



void friendlyDialog::getSelectedReftimes()
{
   if (!reftimeList->count()) return;
   
   QList<QListWidgetItem*> sRList = reftimeList->selectedItems();
   // sRList = selectedReftimeList = the item/items that are selected.
      SelectedModel SM;
      SM.modelName = currentModel;
      selectedModelsList->clear();
      for (int i=0; i < sRList.count(); i++) {
      SM.refTime = sRList.at(i)->text().toStdString();
      selectedModels.push_back(SM); 
      std::string text = SM.modelName + " " + SM.refTime;
      int newCurrent = selectedModelsList->count();
      
      selectedModelsList->addItem(QString::fromStdString(text));
      selectedModelsList->setCurrentRow(newCurrent);
      selectedModelsList->item(newCurrent)->setSelected(true);
      }
}


void friendlyDialog::selectedModelsListClicked(QListWidgetItem * item)
{
   //std::cerr << "selectedModelsListClicked: " << item->text().toStdString() << std::endl;
   int index = selectedModelsList->row(item);
   if (index < 0 || selectedModels.size() == 0) return;
}

void friendlyDialog::deselectClicked()
{
      //std::cerr << "deselectClicked: " <<  std::endl;
   if (selectedModelsList->count()) {
      int row = selectedModelsList->currentRow();
      selectedModelsList->takeItem(row);
   }
}

void friendlyDialog::makeSelectedModelInfo()
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
   for (; it!=smlitems.end(); ++it) {
     //std::cerr << *it << std::endl;
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
       //std::cerr << tmpmodelName << ", " << refTime << ", " << fileName << std::endl;
     } else if (tokens.size() == 1) {
       if (fileNames.size()) {
         for (int i = 0; i < fileNames.size();i++) {
           tmpmodelName = tokens[0];
           std::cerr << tmpmodelName << ", " << refTime << ", " << fileNames[i] << std::endl;
           ModelFileInfo mfi(fileNames[i], refTime);
           modelfileinfos.push_back(mfi);
         }
       }
     }
   }
   //std::cerr << std::endl;
   SelectedModelInfo smi(tmpmodelName,  modelfileinfos);
   modelinfos.push_back(smi);
   m_selectedModelFiles = modelinfos;
}

void friendlyDialog::applyHideClicked()
{
   dHide();
   makeSelectedModelInfo();
   emit dialogApply();
}

void friendlyDialog::applyClicked()
{
  makeSelectedModelInfo();
  emit dialogApply();
}

void friendlyDialog::dHide()
{
   this->hide();
}
