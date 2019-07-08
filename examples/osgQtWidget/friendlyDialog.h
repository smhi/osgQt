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
class friendlyDialog : public QDialog 
{
Q_OBJECT
   

public:
  friendlyDialog(QWidget* parent, ModelManager* mm);
  
  void updateModelList(); //
  void getModel();

protected:
  void closeFriendlyDialogEvent(QCloseEvent*);

private:
  QTreeView* modelList;
  QSortFilterProxyModel* modelFilter;
  QStandardItemModel* modelItems;
  QLineEdit* modelFilterEdit;
//  QListWidget* modelList;
  QListWidget* reftimeList;
  QListWidget* selectedModelsList;
  QString getSelectedModelString(); //getSelectedModelString -> getSelectedModel

  std::string currentModel;
  std::string modelName;
  QWidget* m_parent;
  FieldModelGroupInfo_v m_modelGroups;
  ModelManager* m_mm;

public Q_SLOTS:
  void deselectAll(); //org, deselect All Selected -> deselectAll

private Q_SLOTS:
  void modelListClicked(const QModelIndex& index);
  void filterModels(const QString& filterText);
//  void modelListClicked(QListWidgetItem*);
  void updateReferencetime();
  void reftimeListClicked(QListWidgetItem*);
  void deselectClicked();
//  void helpClicked();
  void applyHideClicked();
  void applyClicked();



Q_SIGNALS:
  void dialogHide();
  void dialogApply();
//  void showSource(const std::string, const std::string = ""); //maybe...

};

#endif
