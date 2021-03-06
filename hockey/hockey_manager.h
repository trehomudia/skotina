#ifndef STATISTICA_H
#define STATISTICA_H

#include <QMainWindow>
#include "team.h"

class QSplitter;
class QStandardItemModel;

class CFootbolManager : public QMainWindow
{
  Q_OBJECT

public:
  explicit CFootbolManager(QWidget *parent = 0);
  ~CFootbolManager();
  void Do();

public slots:
  void onChecked(const QModelIndex & index);
  void onClicked();

private:
  class PrivateData;
  PrivateData* m_pData;

  void FormDataTeams();
  void FormRates();
  void AnalizeCommonPosition();
  void ShowSource();
  void AddValues(const QString& tableName);
  CWidget* AddTable(const QString& tableName);
  QVector<QString> GetSortNextNames(QString champName);
};

#endif // STATISTICA_H
