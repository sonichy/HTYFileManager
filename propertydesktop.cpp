#include "propertydesktop.h"
#include "ui_propertydesktop.h"
#include <QFileDialog>
#include <QProcess>
#include <QDebug>

PropertyDesktop::PropertyDesktop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertyDesktop)
{
    ui->setupUi(this);
    ui->gridLayout->itemAtPosition(0,0)->setAlignment(Qt::AlignCenter);
    ui->lineEditPathDesktop->setVisible(false);
    ui->lineEditIcon->setVisible(false);
    setFixedSize(400,300);
    connect(ui->pushButtonIcon,SIGNAL(clicked(bool)),this,SLOT(changeIcon()));
    connect(ui->pushButtonPath,SIGNAL(clicked(bool)),this,SLOT(desktopPath()));
}

PropertyDesktop::~PropertyDesktop()
{
    delete ui;
}

void PropertyDesktop::changeIcon()
{
    QString pathIcon = QFileDialog::getOpenFileName(this,"打开图片", QFileInfo(ui->lineEditExec->text()).absolutePath(), "图片文件(*.jpg *.jpeg *.png *.bmp *.svg)");
    if(pathIcon.length() != 0){
        ui->pushButtonIcon->setIcon(QIcon(pathIcon));
        ui->lineEditIcon->setText(pathIcon);
    }
}

void PropertyDesktop::desktopPath()
{
    QProcess *proc = new QProcess;
    QString cmd = QDir::currentPath() + "/HTYFileManager " + QFileInfo(ui->lineEditExec->text()).absolutePath();
    qDebug() << cmd;
    proc->start(cmd);
}

void PropertyDesktop::saveDesktop(){
    qDebug() << "saveDesktop";
    QString strAll;
    QStringList strList;
    QFile readFile(ui->lineEditPathDesktop->text());
    if(readFile.open((QIODevice::ReadOnly|QIODevice::Text)))
    {
        QTextStream stream(&readFile);
        strAll=stream.readAll();
    }
    readFile.close();
    QFile writeFile(ui->lineEditPathDesktop->text());
    if(writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream stream(&writeFile);
        strList=strAll.split("\n");
        for(int i=0;i<strList.count();i++)
        {
            if(strList.at(i).contains("name=",Qt::CaseInsensitive))
            {
                QString tempStr="Name="+ui->lineEditName->text();
                stream<<tempStr<<'\n';
                continue;
            }
            if(strList.at(i).contains("icon=",Qt::CaseInsensitive))
            {
                QString tempStr="Icon="+ui->lineEditIcon->text();
                stream<<tempStr<<'\n';
                continue;
            }
            if(strList.at(i).contains("exec=",Qt::CaseInsensitive))
            {
                QString tempStr="Exec="+ui->lineEditExec->text();
                stream<<tempStr<<'\n';
                continue;
            }
            if(strList.at(i).contains("path=",Qt::CaseInsensitive))
            {
                QString tempStr="Path="+ui->lineEditPath->text();
                stream<<tempStr<<'\n';
                continue;
            }
            if(strList.at(i).contains("comment=",Qt::CaseInsensitive))
            {
                QString tempStr="Comment="+ui->lineEditComment->text();
                stream<<tempStr<<'\n';
                continue;
            }
            stream<<strList.at(i)<<'\n';
        }
    }
    writeFile.close();
}
