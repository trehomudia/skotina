#include "footbol_manager.h"
#include "storage.h"
#include "ui_footbol_manager.h"
#include "table_view.h"
#include <QStandardItemModel>

const int deltaConcurent = 3;
const int confidentialSeasons = 2;
const int parityColumn = 0;

class CFootbolManager::PrivateData
{
public:
  PrivateData(){}
  ~PrivateData(){}

  QMap<QString, Championat> championats;
  QMap<QString, NextTur> nextTurs;
  Ui::statistica ui;
  CStorage storage;
};

CFootbolManager::CFootbolManager(QWidget *parent)
  : QMainWindow(parent)
{
  m_pData = new PrivateData();
  m_pData->ui.setupUi(this);
}

CFootbolManager::~CFootbolManager()
{
  delete m_pData;
}

void CFootbolManager::Do()
{
  m_pData->championats = m_pData->storage.ReadFiles(confidentialSeasons);
  m_pData->nextTurs = m_pData->storage.ReadNext();
  FormDataTeams();
  FormRates();
  ShowSource();
}

void CFootbolManager::FormRates()
{
  foreach(QString champName, m_pData->championats.keys())
  {
    if (m_pData->storage.RateIsEmpty(champName))
    {
      //в случае отсутствия или пустоты файлов формирование номеров ставок
      for(int i = 0; i < m_pData->championats[champName].count(); ++i)
        m_pData->championats[champName][i].FindCurrentCashParity();
      QVector<QString> list = GetSortNextNames(champName);

      for(int i = 0; i < list.count(); ++i)
      {
        if (m_pData->championats[champName][i].GetName() == list[i])
        {
          m_pData->storage.AddTeam(m_pData->championats[champName][i], champName, PARITY);
        }
        else
        {
          m_pData->championats[champName][i].SetName(list[i]);
          m_pData->storage.AddTeam(m_pData->championats[champName][i], champName, PARITY);
        }
      }
    }
    else
    {
      QVector<QString> list = GetSortNextNames(champName);
      AddValues(champName);
      for(int i = 0; i < list.count(); ++i)
      {
        int numCashParity = m_pData->storage.ReadCurrentRate(list[i], champName, PARITY);
        if (-1 != numCashParity && list[i] == m_pData->championats[champName][i].GetName())
        {
          m_pData->championats[champName][i].SetCurrentCashParity(numCashParity);
        }
        else
        {
          for(int j = 0; j < m_pData->championats[champName].count(); ++j)
            m_pData->championats[champName][j].ExchengeConcurents(m_pData->championats[champName][i].GetName(), list[i]);

          m_pData->storage.ExchangeName(m_pData->championats[champName][i].GetName(), champName, PARITY, list[i]);
          m_pData->championats[champName][i].SetName(list[i]);
          m_pData->championats[champName][i].SetCurrentCashParity(m_pData->storage.ReadCurrentRate(m_pData->championats[champName][i].GetName(), champName, PARITY));
        }
      }

    }
  }
}

void CFootbolManager::AddValues(const QString& tableName)
{
  NextTur tur = m_pData->storage.ReadPlays(tableName);
  int count = 0;
  for (int i = 0; i < tur.count(); ++i)
  {
    foreach(CTeam team, m_pData->championats[tableName])
    {
      Season s = team.GetSeasons().last();
      if (tur[i].first == team.GetName() && tur[i].second == s.last().opponent)
      {
        count++;
      }
    }
  }

  if (count != tur.count())
    return;

  for (int i = 0; i < tur.count(); ++i)
  {
    foreach(CTeam team, m_pData->championats[tableName])
    {
      Season s = team.GetSeasons().last();
      if (tur[i].first == team.GetName() && tur[i].second == s.last().opponent && team.Concurents().contains(tur[i].second))
      {
        if(s.last().point == 1)
        {
          m_pData->storage.ExchangeRate(tur[i].first, tableName, PARITY, 0);
          m_pData->storage.ExchangeRate(tur[i].second, tableName, PARITY, 0);
        }
        else
        {
          m_pData->storage.ExchangeRate(tur[i].first, tableName, PARITY, m_pData->storage.ReadCurrentRate(tur[i].first, tableName, PARITY) + 1);
          m_pData->storage.ExchangeRate(tur[i].second, tableName, PARITY, m_pData->storage.ReadCurrentRate(tur[i].second, tableName, PARITY) + 1);
        }
      }
    }
  }
  int g = 9;
}

void CFootbolManager::AnalizeCommonPosition()
{
  //сортировка команд по очкам
  foreach(QString champName, m_pData->championats.keys())
  {
    for(int i = 0; i < m_pData->championats[champName].count(); ++i)
    {
      for(int j = i; j < m_pData->championats[champName].count(); j++)
      {
        if (m_pData->championats[champName][j].PointsCommon() > m_pData->championats[champName][i].PointsCommon())
        {
          CTeam team = m_pData->championats[champName][j];
          m_pData->championats[champName][j] = m_pData->championats[champName][i];
          m_pData->championats[champName][i] = team;
        }
        else if(m_pData->championats[champName][j].PointsCommon() == m_pData->championats[champName][i].PointsCommon() &&
                m_pData->championats[champName][j].Differince() > m_pData->championats[champName][i].Differince())
        {
          CTeam team = m_pData->championats[champName][j];
          m_pData->championats[champName][j] = m_pData->championats[champName][i];
          m_pData->championats[champName][i] = team;
        }
      }
    }
  }

  //поиск конкурентов
  foreach(QString champName, m_pData->championats.keys())
  {
    for(int i = 0; i < m_pData->championats[champName].count(); ++i)
    {
      QList<QString> con_s;
      for(int j = 0; j < m_pData->championats[champName].count(); j++)
      {
        if (abs(j - i) <= deltaConcurent)
        {
          con_s << m_pData->championats[champName][j].GetName();
        }
      }
      con_s.removeAll(m_pData->championats[champName][i].GetName());
      m_pData->championats[champName][i].SetConcurents(con_s.toVector());
    }
  }
}

void CFootbolManager::FormDataTeams()
{
  foreach(QString champName, m_pData->championats.keys())
    for(int i = 0; i < m_pData->championats[champName].count(); ++i)
      m_pData->championats[champName][i].FormData();

  AnalizeCommonPosition();

  foreach(QString champName, m_pData->championats.keys())
    for(int i = 0; i < m_pData->championats[champName].count(); ++i)
      m_pData->championats[champName][i].FormDataCommon();

  QVector<int> no_paritysCommon;
  foreach(QString str, m_pData->championats.keys())
  {
    foreach(CTeam team, m_pData->championats.value(str))
    {
      no_paritysCommon << team.NoParityesCommon();
    }
  }

  qSort(no_paritysCommon);

  int max = 14;
  int overC = 0;
  foreach(int npc, no_paritysCommon)
  {
    if(npc > max)
      overC++;
  }

  double pC = static_cast<double>(overC) / static_cast<double>(no_paritysCommon.count());

  QMap<int, int> map1;
  foreach(int v, no_paritysCommon)
  {
    if (!map1.contains(v))
      map1.insert(v, 0);
    map1[v]++;
  }

  int cash = 0;
  int koef = 3;
  for(int i = 1; i < map1.lastKey(); ++i)
  {
    if(!map1.contains(i))
      map1.insert(i, 0);
  }

//  foreach(int count, map1.keys())
//  {
//    int plus = 0;
//    int minus = 0;
//    int mCount = 0;

//    while((count - cashList.count() * mCount) >= cashList.count())
//      mCount++;

//    plus = map1.value(count) * cashList.value(count - cashList.count() * mCount) * koef;

//    while(mCount >= 0)
//    {
//      for(int i = 0; i <= count - cashList.count() * mCount; ++i)
//        minus += map1.value(count) * cashList.value(i);
//      --mCount;
//    }
//    cash += plus - minus;
//  }

//  int g = cashList.count();
//  int k = g;
}

QVector<QString> CFootbolManager::GetSortNextNames(QString champName)
{
  QVector<QString> names;
  for(int i = 0; i < m_pData->nextTurs.value(champName).count(); ++i)
  {
    names << m_pData->nextTurs.value(champName)[i].first;
    names << m_pData->nextTurs.value(champName)[i].second;
  }

  QVector<QString> sortNames;
  sortNames.resize(names.size());
  for(int i = 0; i < names.count(); ++i)
  {
    if (names.contains(m_pData->championats[champName][i].GetName()))
      sortNames[i] = m_pData->championats[champName][i].GetName();
  }

  foreach(QString name, sortNames)
  {
    if(names.contains(name))
      names.remove(names.indexOf(name));
  }

  for(int i = 0; i < sortNames.count(); ++i)
  {
    if (sortNames[i].isEmpty())
    {
      sortNames[i] = names.first();
      names.remove(0);
    }
  }

  return sortNames;
}

void CFootbolManager::ShowSource()
{
  foreach(QString champName, m_pData->championats.keys())
  {
    CWidget* table = AddTable(champName);
    CStandardItemModel* modelCommon = table->TableCommon();
    modelCommon->SetColumns(GetSortNextNames(champName));
    modelCommon->setVerticalHeaderLabels(QStringList() << tr("Ничьи"));
    foreach(CTeam team, m_pData->championats.value(champName))
    {
      //table->PrintCommonResult(team, NO_PARITY);
//      model->PrintCommonResult(team, parityColumn, PARITY);
      modelCommon->PrintCommonResult(team.GetName(), parityColumn, team.CashParityPosition());
    }

    CStandardItemModel* modelResult = table->TableResult();
    modelResult->SetColumns(QVector<QString>() << QString("Команда1") << QString("Команда2") << QString("Ничья")/* << QString("ХНичья")*/);
    for(int i = 0; i < m_pData->nextTurs.value(champName).count(); ++i)
    {
      QColor color;
      if(NOTREPORT == m_pData->storage.Report(champName, m_pData->nextTurs.value(champName)[i].first) && NOTREPORT == m_pData->storage.Report(champName, m_pData->nextTurs.value(champName)[i].second))
        color = Qt::green;
      else
        color = Qt::red;

      QStandardItem* item1 = new QStandardItem(m_pData->nextTurs[champName][i].first);
      item1->setBackground(color);
      modelResult->setItem(i, 0, item1);


      QStandardItem* item2 = new QStandardItem(m_pData->nextTurs.value(champName)[i].second);
      item2->setBackground(color);
      modelResult->setItem(i, 1, item2);

      CTeam team1;
      foreach(CTeam t, m_pData->championats.value(champName))
      {
        if (m_pData->nextTurs.value(champName)[i].first == t.GetName())
          team1 = t;
      }

      QStandardItem* item3 = new QStandardItem();
      item3->setBackground(color);
      if(team1.Concurents().contains(m_pData->nextTurs.value(champName)[i].second))
      {
        CTeam team2;
        foreach(CTeam t, m_pData->championats.value(champName))
        {
          if (m_pData->nextTurs.value(champName)[i].second == t.GetName())
            team2 = t;
        }
        item3->setData(QString::number(GetCash(team1.CashParityPosition(), PARITY) + GetCash(team2.CashParityPosition(), PARITY)), Qt::DisplayRole);
      }
      else
      {
        item3->setData(QString("--"), Qt::DisplayRole);
      }
      modelResult->setItem(i, 2, item3);


//      QStandardItem* item4 = new QStandardItem(QString());
//      item4->setBackground(color);
//      modelResult->setItem(i, 3, item4);

    }

  }
}

void CFootbolManager::onChecked(const QModelIndex &index)
{
  CWidget* widget = qobject_cast<CWidget*>(sender());
  CStandardItemModel* modelResult = widget->TableResult();
  for(int i = 0; i < m_pData->nextTurs.value(widget->objectName()).count(); ++i)
  {
    if (m_pData->nextTurs.value(widget->objectName())[i].first == modelResult->item(index.row(), 0)->data(Qt::DisplayRole).toString()
        && m_pData->nextTurs.value(widget->objectName())[i].second == modelResult->item(index.row(), 1)->data(Qt::DisplayRole).toString())
    {
      m_pData->storage.Reported(widget->objectName(), modelResult->item(index.row(), 0)->data(Qt::DisplayRole).toString(), REPORT);
      m_pData->storage.Reported(widget->objectName(), modelResult->item(index.row(), 1)->data(Qt::DisplayRole).toString(), REPORT);
      for(int j = 0; j < modelResult->columnCount(); ++j)
        modelResult->item(i, j)->setBackground(Qt::red);
    }
  }
}

void CFootbolManager::onClicked()
{
  m_pData->storage.CreatePlays(sender()->objectName(), m_pData->nextTurs.value(sender()->objectName()));
}

CWidget* CFootbolManager::AddTable(const QString& tableName)
{
  CWidget* widget = new CWidget(this);
  widget->setObjectName(tableName);
  connect(widget, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onChecked(const QModelIndex&)));
  connect(widget, SIGNAL(clicked()), this, SLOT(onClicked()));
  connect(&m_pData->storage, SIGNAL(AllChecked(bool)), widget, SIGNAL(AllChecked(bool)));
  m_pData->ui.tabWidget->addTab(widget, tableName);
  return widget;
}
