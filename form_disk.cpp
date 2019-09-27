#include "form_disk.h"
#include "ui_form_disk.h"
#include <QDebug>

Form_disk::Form_disk(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_disk)
{
    ui->setupUi(this);
    ui->label_icon->setPixmap(QIcon::fromTheme("drive-harddisk").pixmap(70,70));
}

Form_disk::~Form_disk()
{
    delete ui;
}

void Form_disk::init()
{
    qint64 bytesUsed = bytesTotal - bytesFree;
    ui->label_bytes->setText(BS(bytesUsed) + " / " + BS(bytesTotal));
    ui->progressBar->setRange(0, 100);
    int value = 100 * bytesUsed / bytesTotal;
    //qDebug() << value;
    ui->progressBar->setValue(value);
    QString style;
    if(value < 60){
        style = "QProgressBar { border: 1px solid gray; border-radius: 5px; background: white; }"
                "QProgressBar::chunk { background-color: dodgerblue; }";
    }else if(value < 80){
        style = "QProgressBar { border: 1px solid gray; border-radius: 5px; background: white; }"
                "QProgressBar::chunk { background-color: orange; }";
    }else{
        style = "QProgressBar { border: 1px solid gray; border-radius: 5px; background: white; }"
                "QProgressBar::chunk { background-color: red; }";
    }
    ui->progressBar->setStyleSheet(style);
}

QString Form_disk::BS(qint64 b)
{
    QString s = "";
    if (b > 999999999) {
        s = QString::number(b/(1024*1024*1024)) + " GB";
    } else {
        if (b > 999999){
            s = QString::number(b/(1024*1024)) + " MB";
        } else {
            if (b > 999) {
                s = QString::number(b/1024) + " KB";
            } else {
                s = QString::number(b)+" B";
            }
        }
    }
    return s;
}