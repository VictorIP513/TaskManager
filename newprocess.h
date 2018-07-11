#ifndef NEWPROCESS_H
#define NEWPROCESS_H

#include <cstdlib>
#include <QMainWindow>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include <QProcess>

namespace Ui {
class NewProcess;
}

class NewProcess : public QMainWindow
{
    Q_OBJECT

public:
    explicit NewProcess(QWidget *parent = 0);
    ~NewProcess();

private slots:
    void on_pushButton_clicked();
    void on_lineEdit_returnPressed();
    void on_lineEdit_textChanged(const QString &arg1);

private:
    Ui::NewProcess *ui;
    QTemporaryFile *temporaryFile;
};

#endif // NEWPROCESS_H
