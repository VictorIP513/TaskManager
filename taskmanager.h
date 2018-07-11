#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <pthread.h>
#include "newprocess.h"
#include <cstdlib>
#include <cmath>
#include <QMainWindow>
#include <QTemporaryFile>
#include <QTimer>
#include <QTextStream>

namespace Ui {
class TaskManager;
}

class TaskManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit TaskManager(QWidget *parent = 0);
    ~TaskManager();

protected:
     void closeEvent(QCloseEvent *event);

private slots:
    void updateTimer();
    void on_tableWidget_itemSelectionChanged();
    void on_killProcess_pushButton_clicked();
    void on_newProcess_pushButton_clicked();
    void on_advancedInformation_checkBox_clicked();
    void on_myProcesses_radioButton_clicked();
    void on_allProcesses_radioButton_clicked();

private:
    Ui::TaskManager *ui;
    friend void *upd(void *obj_param);
    void getUserName();
    void getCpuLoad();
    void getMemLoad();
    void getCpuCount();
    void getOsName();
    void getProcessorModel();
    void getUserNameFromUID();
    void fillData();
    void getProcMemoryUsed();
    void deleteProcFromLists(QString procPID);
    void fillProcInformationToTableWidget();
    void setupTableWidget();
    void setProgressBars();
    void fillSystemInformation();
    QString getProcNameFromCMD(QString cmd);
    QTemporaryFile *temporaryFile;
    QTimer *timer;
    pthread_t thread;
    QStringList listUID;
    QStringList listPID;
    QStringList listCMD;
    QStringList listUsernames;
    QStringList listProcNames;
    QStringList listCpuLoad;
    QStringList listMemoryUsed;
    int cpuCount;
    float cpuLoad;
    int maxMemory;
    int memoryAvailable;
    QString username;
    QString osName;
    QString processorModel;
};

#endif // TASKMANAGER_H
