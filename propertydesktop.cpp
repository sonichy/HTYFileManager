#include "propertydesktop.h"
#include "ui_propertydesktop.h"
#include <QFileDialog>
#include <QProcess>
#include <QDebug>
#include <QSettings>

PropertyDesktop::PropertyDesktop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertyDesktop)
{
    ui->setupUi(this);
    //setFixedSize(400,350);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("保存");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");
    //ui->gridLayout->itemAtPosition(0,0)->setAlignment(Qt::AlignCenter);
    connect(ui->pushButtonIcon,SIGNAL(clicked(bool)),this,SLOT(changeIcon()));

    QAction *action_exec = new QAction(this);
    action_exec->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    action_exec->setToolTip("选择文件");
    connect(action_exec,SIGNAL(triggered(bool)),this,SLOT(changeExec()));
    ui->lineEditExec->addAction(action_exec,QLineEdit::TrailingPosition);
    connect(ui->lineEditExec,SIGNAL(textChanged(QString)),this,SLOT(lineEditExecTextChanged(QString)));

    QAction *action_path = new QAction(this);
    action_path->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    action_path->setToolTip("打开路径");
    connect(action_path,SIGNAL(triggered(bool)),this,SLOT(openPath()));
    ui->lineEditPath->addAction(action_path,QLineEdit::TrailingPosition);

}

PropertyDesktop::~PropertyDesktop()
{
    delete ui;
}

void PropertyDesktop::changeIcon()
{
    QString newpath = QFileDialog::getOpenFileName(this,"选择图片", QFileInfo(ui->lineEditExec->text()).absolutePath(), "图片文件(*.jpg *.jpeg *.png *.bmp *.svg *.ico)");
    if (newpath != "") {
        ui->pushButtonIcon->setIcon(QIcon(newpath));
        iconPath = newpath;
    }
}

void PropertyDesktop::changeExec()
{
    QString newpath = QFileDialog::getOpenFileName(this,"选择可执行文件", QFileInfo(ui->lineEditExec->text()).absolutePath());
    if (newpath != "") {
        ui->lineEditExec->setText(newpath);
    }
}

void PropertyDesktop::openPath()
{
    QProcess *proc = new QProcess;
    QString cmd = QDir::currentPath() + "/HTYFileManager " + QFileInfo(ui->lineEditExec->text()).absolutePath();
    qDebug() << cmd;
    proc->start(cmd);
}

void PropertyDesktop::saveDesktop(){
    qDebug() << "saveDesktop";    
    // QSettings 字段空格会转码，暂时弃用
//    writeSettings(filePath, "Desktop Entry", "Name", ui->lineEditName->text());
//    writeSettings(filePath, "Desktop Entry", "Exec", ui->lineEditExec->text());
//    writeSettings(filePath, "Desktop Entry", "Path", ui->lineEditPath->text());
//    writeSettings(filePath, "Desktop Entry", "Icon", iconPath);
//    writeSettings(filePath, "Desktop Entry", "Comment", ui->lineEditComment->text());
//    writeSettings(filePath, "Desktop Entry", "Categories", ui->lineEditCategories->text());
    QString strAll;
    QStringList strList;
    QFile readFile(filePath);
    if (readFile.open((QIODevice::ReadOnly | QIODevice::Text))) {
        QTextStream stream(&readFile);
        strAll = stream.readAll();
    }
    readFile.close();
    QFile writeFile(filePath);
    if (writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&writeFile);
        strList = strAll.split("\n");
        for (int i=0; i<strList.count(); i++) {
            if (strList.at(i).contains("Name=", Qt::CaseInsensitive)) {
                QString tempStr = "Name=" + ui->lineEditName->text();
                stream << tempStr << "\n";
            } else if (strList.at(i).contains("Icon=", Qt::CaseInsensitive)) {
                QString tempStr = "Icon=" + iconPath;
                stream << tempStr << "\n";
            } else if (strList.at(i).contains("Exec=", Qt::CaseInsensitive)) {
                QString tempStr = "Exec=" + ui->lineEditExec->text();
                stream << tempStr << "\n";
            } else if (strList.at(i).contains("Path=", Qt::CaseInsensitive)) {
                QString tempStr = "Path=" + ui->lineEditPath->text();
                stream << tempStr << "\n";
            } else if (strList.at(i).contains("Comment=", Qt::CaseInsensitive)) {
                QString tempStr = "Comment=" + ui->lineEditComment->text();
                stream << tempStr << "\n";
            } else if (strList.at(i).contains("Categories=", Qt::CaseInsensitive)) {
                QString tempStr = "Categories=" + ui->lineEditCategories->text();
                stream << tempStr << "\n";
            } else {
                if (strList.at(i) != "")
                    stream << strList.at(i) << "\n";
            }
        }
    }
    writeFile.close();
}

void PropertyDesktop::writeSettings(QString path, QString group, QString key, QString value)
{
    QSettings *settings = new QSettings(path, QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    settings->beginGroup(group);
    settings->setValue(key, value);
    settings->endGroup();
}

void PropertyDesktop::lineEditExecTextChanged(QString newpath)
{
    ui->lineEditPath->setText(QFileInfo(newpath).absolutePath());
}
