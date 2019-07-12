//friendlyDialog.h 
//used for the abstractation of file/model loading
//friendly ~ abstractation of model names, path and reftime/time
#ifndef _FRIENDLYDIALOG_H
#define _FRIENDLYDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include "osgModelManager.h"
#include <map>
/*
class QPushButton;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QModelIndex;
class QSortFilterProxyModel;
class QStandardItemModel;
class QTreeView;
class QVBoxLayout;
*/
/**
   \brief data for one model file
*/
struct ModelFileInfo
{
  std::string fileName;
  std::string refTime;
  ModelFileInfo(){;};
  ModelFileInfo(const std::string& fn, const std::string& rt) {fileName=fn; refTime=rt;};
};
typedef std::vector<ModelFileInfo> ModelFileInfo_v;

/**
   \brief data for one model
*/
struct SelectedModelInfo
{
  std::string modelName;
  
  ModelFileInfo_v modelFiles;
  SelectedModelInfo(){;};
  SelectedModelInfo(const std::string& mn, const ModelFileInfo_v& mi) {modelName=mn; modelFiles=mi;};
};
/*
For now, only one model!
*/
typedef std::vector<SelectedModelInfo> SelectedModelInfo_v;

class friendlyDialog : public QDialog 
{
Q_OBJECT

private: struct SelectedModel{
std::string modelName;
std::string refTime;
};

std::map<std::string, std::string> reftime_filename;   

public:
  friendlyDialog(QWidget* parent, ModelManager* mm);
  
  void updateModelList(); //
  void updateDialog();
  void getModel();
  void archiveMode(bool on);
  SelectedModelInfo_v getSelectedModelFiles() {return m_selectedModelFiles;};

protected:
  void closeFriendlyDialogEvent(QCloseEvent*);

private:
  QTreeView* modelList;
  QSortFilterProxyModel* modelFilter;
  QStandardItemModel* modelItems;
  QLineEdit* modelFilterEdit;
  QListWidget* reftimeList;
  QListWidget* selectedModelsList;
  void addModelGroup(int modelGroupIndex);
  bool useArchive; 
  std::string currentModel;
  std::string modelName;
  QWidget* m_parent;
  FieldModelGroupInfo_v m_modelGroups;
  ModelManager* m_mm;
  std::vector<SelectedModel> selectedModels;
  // performance
  std::set<std::string> refTimes;
  std::vector<std::string> fileNames;
  SelectedModelInfo_v m_selectedModelFiles;
  void makeSelectedModelInfo();
public Q_SLOTS:
  void deselectAll();

private Q_SLOTS:
  void modelListClicked(const QModelIndex& index);
  void filterModels(const QString& filterText);
  void getSelectedReftimes();
  void selectedModelsListClicked(QListWidgetItem*);
  void deselectClicked();
//  void helpClicked();
  void applyHideClicked();
  void applyClicked();
  void dHide(); 


Q_SIGNALS:
  void dialogHide();
  void dialogApply();
//  void showSource(const std::string, const std::string = ""); //maybe...

};

#endif
