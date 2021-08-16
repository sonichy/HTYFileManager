#include "propertydesktop.h"
#include "ui_propertydesktop.h"
#include <QFileDialog>
#include <QProcess>
#include <QDebug>
#include <QSettings>
#include <QAction>

PropertyDesktop::PropertyDesktop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertyDesktop)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool);
    setFixedSize(400,400);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("保存");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");
    //ui->gridLayout->itemAtPosition(0,0)->setAlignment(Qt::AlignCenter);
    connect(ui->pushButton_icon, SIGNAL(clicked(bool)), this, SLOT(changeIcon()));

    QAction *action_exec = new QAction(this);
    action_exec->setIcon(QIcon::fromTheme("folder"));
    action_exec->setToolTip("选择文件");
    connect(action_exec, SIGNAL(triggered(bool)), this, SLOT(changeExec()));
    ui->lineEdit_exec->addAction(action_exec,QLineEdit::TrailingPosition);
    connect(ui->lineEdit_exec, SIGNAL(textChanged(QString)), this, SLOT(lineEditExecTextChanged(QString)));

    QAction *action_path = new QAction(this);
    action_path->setIcon(QIcon::fromTheme("folder"));
    action_path->setToolTip("打开路径");
    connect(action_path, SIGNAL(triggered(bool)), this, SLOT(openPath()));
    ui->lineEdit_path->addAction(action_path, QLineEdit::TrailingPosition);

}

PropertyDesktop::~PropertyDesktop()
{
    delete ui;
}

void PropertyDesktop::changeIcon()
{
    QString newpath = QFileDialog::getOpenFileName(this,"选择图片", iconPath, "图片文件(*.jpg *.jpeg *.png *.bmp *.svg *.ico)");
    if (newpath != "") {
        ui->pushButton_icon->setIcon(QIcon(newpath));
        iconPath = newpath;
    }
}

void PropertyDesktop::changeExec()
{
    QString newpath = QFileDialog::getOpenFileName(this,"选择可执行文件", QFileInfo(ui->lineEdit_exec->text()).absolutePath());
    if (newpath != "") {
        ui->lineEdit_exec->setText(newpath);
    }
}

void PropertyDesktop::openPath()
{
    QProcess *proc = new QProcess;
    QString cmd = QApplication::applicationDirPath() + "/HTYFileManager " + ui->lineEdit_path->text();
    qDebug() << cmd;
    proc->start(cmd);
}

void PropertyDesktop::saveDesktop(){
    qDebug() << "saveDesktop";
    QString strAll;
    QStringList SL;
    QFile readFile(filePath);
    if (readFile.open((QIODevice::ReadOnly | QIODevice::Text))) {
        QTextStream stream(&readFile);
        strAll = stream.readAll();
    }
    readFile.close();
    QFile writeFile(filePath);
    if (writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&writeFile);
        SL = strAll.split("\n");
        for (int i=0; i<SL.count(); i++) {
            if (SL.at(i).contains("Name=", Qt::CaseInsensitive)) {
                QString tempStr = "Name=" + ui->lineEdit_name->text();
                stream << tempStr << "\n";
            } else if (SL.at(i).contains("Icon=", Qt::CaseInsensitive)) {
                QString tempStr = "Icon=" + iconPath;
                stream << tempStr << "\n";
            } else if (SL.at(i).contains("Exec=", Qt::CaseInsensitive)) {
                QString tempStr = "Exec=" + ui->lineEdit_exec->text();
                stream << tempStr << "\n";
            } else if (SL.at(i).contains("Path=", Qt::CaseInsensitive)) {
                QString tempStr = "Path=" + ui->lineEdit_path->text();
                stream << tempStr << "\n";
            } else if (SL.at(i).contains("Comment=", Qt::CaseInsensitive)) {
                QString tempStr = "Comment=" + ui->lineEdit_comment->text();
                stream << tempStr << "\n";
            } else if (SL.at(i).contains("Categories=", Qt::CaseInsensitive)) {
                QString tempStr = "Categories=" + ui->lineEdit_categories->text();
                stream << tempStr << "\n";
            } else {
                if (SL.at(i) != "")
                    stream << SL.at(i) << "\n";
            }
        }
        if(!SL.contains("Path=", Qt::CaseInsensitive)){
            stream << "Path=\n";
        }
    }
    writeFile.close();
}

void PropertyDesktop::lineEditExecTextChanged(QString newpath)
{
    ui->lineEdit_path->setText(QFileInfo(newpath).absolutePath());
}