#include "newprocess.h"
#include "ui_newprocess.h"
#include <QDebug>

NewProcess::NewProcess(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NewProcess)
{
    ui->setupUi(this);
    temporaryFile = new QTemporaryFile();
    ui->pushButton->setEnabled(false);
}

NewProcess::~NewProcess()
{
    delete ui;
}

void NewProcess::on_pushButton_clicked()
{
    temporaryFile->open();
    QString command = "dpkg -s " + ui->lineEdit->text() + " > " + temporaryFile->fileName();
    system(command.toStdString().c_str());
    QTextStream textstream(temporaryFile);
    QString str = textstream.readAll();
    temporaryFile->close();
    bool flagError = str.isEmpty();
    if (flagError)
    {
        if (ui->lineEdit->text().contains("/"))
        {
            QFileInfo fileInfo(ui->lineEdit->text());
            if (fileInfo.isExecutable())
            {
                QProcess *process = new QProcess(this);
                process->start(ui->lineEdit->text());
                flagError = false;
                this->close();
            }
        }
    }
    else
    {
        QString command = ui->lineEdit->text() + " & exit > " + temporaryFile->fileName();
        system(command.toStdString().c_str());
        this->close();
    }
    if (flagError)
    {
        QMessageBox::warning(this, "Создать новый процесс",
                             "Данного приложения не существует, проверьте введенные данные.", QMessageBox::Ok);
    }

}

void NewProcess::on_lineEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        ui->pushButton->setEnabled(false);
    }
    else
    {
        ui->pushButton->setEnabled(true);
    }
}

void NewProcess::on_lineEdit_returnPressed()
{
    if (!ui->lineEdit->text().isEmpty())
    {
        on_pushButton_clicked();
    }
}
