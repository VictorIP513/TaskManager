#include "taskmanager.h"
#include "ui_taskmanager.h"

TaskManager::TaskManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TaskManager)
{
    ui->setupUi(this);
    temporaryFile = new QTemporaryFile();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
    getUserName();
    getCpuCount();
    getOsName();
    getProcessorModel();
    getCpuLoad();
    getMemLoad();
    fillData();
    getUserNameFromUID();
    getProcMemoryUsed();
    setProgressBars();
    fillSystemInformation();
    setupTableWidget();
    fillProcInformationToTableWidget();
    updateTimer();
}

TaskManager::~TaskManager()
{
    delete ui;
}

void *upd(void *obj_param)
{
    TaskManager *_class = ((TaskManager *)obj_param);
    _class->listCMD.clear();
    _class->listCpuLoad.clear();
    _class->listMemoryUsed.clear();
    _class->listPID.clear();
    _class->listProcNames.clear();
    _class->listUID.clear();
    _class->listUsernames.clear();
    _class->getCpuLoad();
    _class->getMemLoad();
    _class->fillData();
    _class->getUserNameFromUID();
    _class->getProcMemoryUsed();
    _class->setProgressBars();
    _class->fillSystemInformation();
    _class->setupTableWidget();
    _class->fillProcInformationToTableWidget();
    return nullptr;
}

void TaskManager::updateTimer()
{
    pthread_join(thread, NULL);
    pthread_create(&thread, NULL, &upd, this);
    timer->start((int)(ui->doubleSpinBox->value() * 1000));
}

void TaskManager::setProgressBars()
{
    ui->cpu_progressBar->setValue((int)cpuLoad);
    ui->mem_progressBar->setMaximum(maxMemory);
    ui->mem_progressBar->setValue(maxMemory - memoryAvailable);
}

void TaskManager::setupTableWidget()
{
    if (ui->myProcesses_radioButton->isChecked())
    {
        int counter = 0;
        for (int i = 0; i < listPID.length(); ++i)
        {
            if (listUsernames.at(i) == username)
            {
                ++counter;
            }
        }
        ui->tableWidget->setRowCount(counter);
    }
    else
    {
        ui->tableWidget->setRowCount(listPID.length());
    }
    ui->tableWidget->setColumnCount(5);
    QStringList labels;
    labels << "Название процесса" << "ID" << "Пользователь" << "%ЦП" << "Память";
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

void TaskManager::fillProcInformationToTableWidget()
{
    QTableWidgetItem *item;
    for (int i = 0, row_count = 0; i < listPID.length(); ++i)
    {
        if (!ui->allProcesses_radioButton->isChecked() && listUsernames.at(i) != username)
        {
            continue;
        }
        item = new QTableWidgetItem(listProcNames.at(i));
        if (ui->advancedInformation_checkBox->isChecked())
        {
            item->setToolTip(listCMD.at(i));
        }
        ui->tableWidget->setItem(row_count, 0, item);
        item = new QTableWidgetItem(listPID.at(i));
        ui->tableWidget->setItem(row_count, 1, item);
        if (ui->advancedInformation_checkBox->isChecked())
        {
            item = new QTableWidgetItem(listUsernames.at(i) + " (" + listUID.at(i) + ")");
        }
        else
        {
            item = new QTableWidgetItem(listUsernames.at(i));
        }
        ui->tableWidget->setItem(row_count, 2, item);
        double cpuLoad = QString(listCpuLoad.at(i)).toDouble() / cpuCount;
        cpuLoad = round(cpuLoad * 10) / 10;
        item = new QTableWidgetItem(QString::number(cpuLoad) + "%");
        ui->tableWidget->setItem(row_count, 3, item);
        if (listMemoryUsed.at(i) == "")
        {
            item = new QTableWidgetItem("Н/Д");
        }
        else
        {
            double memoryUsed = QString(listMemoryUsed.at(i)).toDouble() / 1024;
            memoryUsed = round(memoryUsed * 10) / 10;
            item = new QTableWidgetItem(QString::number(memoryUsed) + "мб");
        }
        ui->tableWidget->setItem(row_count, 4, item);
        ++row_count;
    }
    ui->tableWidget->resizeColumnsToContents();
}

void TaskManager::getUserName()
{
    temporaryFile->open();
    QString command = "id -u -n > " + temporaryFile->fileName();
    system(command.toStdString().c_str());
    QByteArray username = temporaryFile->readAll();
    temporaryFile->close();
    this->username = QString(username).remove(QRegExp("[\n]"));
}

void TaskManager::getCpuLoad()
{
    temporaryFile->open();
    QString command = "ps aux | awk '{s += $3} END {print s \"%\"}' > " + temporaryFile->fileName();
    system(command.toStdString().c_str());
    QByteArray cpuLoad = temporaryFile->readAll();
    temporaryFile->close();
    QString strCpuLoad = QString(cpuLoad).remove(QRegExp("%[\n]"));
    this->cpuLoad = strCpuLoad.toFloat() / cpuCount;
}

void TaskManager::getMemLoad()
{
    QFile file("/proc/meminfo");
    file.open(QIODevice::ReadOnly);
    QTextStream textStream(&file);
    QString maxMemory = textStream.readLine();
    textStream.readLine();
    QString memoryAvailable = textStream.readLine();
    file.close();
    maxMemory.remove(" ");
    maxMemory.remove("MemTotal:");
    maxMemory.remove("kB");
    memoryAvailable.remove(" ");
    memoryAvailable.remove("MemAvailable:");
    memoryAvailable.remove("kB");
    this->maxMemory = maxMemory.toInt();
    this->memoryAvailable = memoryAvailable.toInt();
}

void TaskManager::getCpuCount()
{
    temporaryFile->open();
    QString command = "lscpu > " + temporaryFile->fileName();
    system(command.toStdString().c_str());
    QTextStream textStream(temporaryFile);
    while (!textStream.atEnd())
    {
        QString str = textStream.readLine();
        if (str.contains("CPU(s):"))
        {
            str.remove("CPU(s):");
            str.remove(QRegExp("[ ]"));
            cpuCount = str.toInt();
            break;
        }
    }
    temporaryFile->close();
}

void TaskManager::getUserNameFromUID()
{
    foreach (const QString &str, listUID)
    {
        temporaryFile->open();
        QString command = "getent passwd | awk -F: '$3 == " + str + " { print $1 }' > " + temporaryFile->fileName();
        system(command.toStdString().c_str());
        QTextStream textStream(temporaryFile);
        listUsernames << textStream.readLine();
        temporaryFile->close();
    }
}

QString TaskManager::getProcNameFromCMD(QString cmd)
{
    if (cmd.at(0) == "/")
    {
        QStringList list = cmd.split(" -");
        QStringList list2 = QString(list.first()).split(" /");
        QStringList list3 = QString(list2.first()).split("/");
        return list3.last();
    }
    QStringList list = cmd.split(" ");
    QString name  = list.at(0);
    if (name.at(0) == "[" && name.at(name.length() - 1) == "]")
    {
        name.remove(0, 1);
        name.remove(name.length() - 1, 1);
    }
    return name;
}

void TaskManager::getProcMemoryUsed()
{
    foreach (const QString &str, listPID)
    {
        QFile file("/proc/" + str + "/status");
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream textStream(&file);
            QString currentStr;
            do
            {
                currentStr = textStream.readLine();
                if (currentStr.contains("RssAnon:"))
                {
                    currentStr.remove("RssAnon:");
                    currentStr.remove(" ");
                    currentStr.remove("\t");
                    currentStr.remove("kB");
                    break;
                }
            }
            while (!currentStr.isNull());
            listMemoryUsed << currentStr;
            file.close();
        }
        else
        {
            deleteProcFromLists(str);
        }
    }
}

void TaskManager::deleteProcFromLists(QString procPID)
{
    int index = listPID.indexOf(procPID);
    listPID.removeAt(index);
    listCMD.removeAt(index);
    listCpuLoad.removeAt(index);
    listMemoryUsed.removeAt(index);
    listProcNames.removeAt(index);
    listUID.removeAt(index);
    listUsernames.removeAt(index);
}

void TaskManager::fillData()
{
    temporaryFile->open();
    QString command = "ps -eo uid,pid,pcpu,cmd > " + temporaryFile->fileName();
    system(command.toStdString().c_str());
    QTextStream textStream(temporaryFile);
    QStringList data;
    while (!textStream.atEnd())
    {
        data << textStream.readLine();
    }
    temporaryFile->close();
    data.removeFirst();
    QStringList list;
    foreach (const QString &str, data)
    {
        list = str.split(" ");
        list.removeAll("");
        listUID << list.at(0);
        listPID << list.at(1);
        listCpuLoad << list.at(2);
        QString procCommand = "";
        for (int i = 3; i < list.length(); ++i)
        {
            procCommand += list.at(i) + " ";
        }
        procCommand.remove(procCommand.length() - 1, procCommand.length() - 1);
        listCMD << procCommand;
        listProcNames << getProcNameFromCMD(procCommand);
    }
    listProcNames.replace(0, "systemd");
}

void TaskManager::getOsName()
{
    temporaryFile->open();
    QString command = "cat /etc/issue > " + temporaryFile->fileName();
    system(command.toStdString().c_str());
    QByteArray osName = temporaryFile->readAll();
    temporaryFile->close();
    this->osName = QString(osName).remove(" \\n \\l\n\n");
}

void TaskManager::getProcessorModel()
{
    QFile file("/proc/cpuinfo");
    file.open(QIODevice::ReadOnly);
    QTextStream textStream(&file);
    QString str;
    do
    {
        str = textStream.readLine();
        if (str.contains("model name"))
        {
            str.remove("model name\t: ");
            break;
        }
    }
    while (!str.isNull());
    file.close();
    this->processorModel = str;
}

void TaskManager::fillSystemInformation()
{
    ui->os_label->setText("Операционная система: " + osName);
    ui->username_label->setText("Текущий пользователь: " + username);
    ui->processorModel_label->setText("Процессор: " + processorModel + " , " + QString::number(cpuCount) + " ядер");
    ui->totalMemory_label->setText("Всего памяти: " + QString::number((int)(maxMemory / 1024)) + "мб");
    ui->usedMemory_label->setText("Используемая память: " + QString::number((int)((maxMemory - memoryAvailable) / 1024)) + "мб");
    ui->availableMemory_label->setText("Доступно памяти: " + QString::number((int)(memoryAvailable / 1024)) + "мб");
}

void TaskManager::on_tableWidget_itemSelectionChanged()
{
    ui->killProcess_pushButton->setEnabled(true);
}

void TaskManager::on_killProcess_pushButton_clicked()
{
    QTableWidgetItem *item = ui->tableWidget->selectedItems().value(1);
    QString command = "kill -9 " + item->text();
    system(command.toStdString().c_str());
}

void TaskManager::on_newProcess_pushButton_clicked()
{
    NewProcess *form = new NewProcess(this);
    form->show();
}

void TaskManager::on_advancedInformation_checkBox_clicked()
{
    updateTimer();
}

void TaskManager::on_myProcesses_radioButton_clicked()
{
    updateTimer();
}

void TaskManager::on_allProcesses_radioButton_clicked()
{
    updateTimer();
}

void TaskManager::closeEvent(QCloseEvent *event)
{
    timer->stop();
    pthread_join(thread, NULL);
}
