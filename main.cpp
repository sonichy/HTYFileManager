#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>

// 日志
void writeLog(QString s){
    QDateTime currentDateTime = QDateTime::currentDateTime();
    s = "[" + currentDateTime.toString("yyyy/MM/dd HH:mm:ss") + "] " + s + "\n";
    QFile file(QApplication::applicationDirPath() + "/log.txt");
    if (file.open(QFile::WriteOnly | QIODevice::Append)) {
        file.write(s.toUtf8());
        file.close();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("HTY");
    a.setApplicationName("HTYFileManager");
    qSetMessagePattern("[ %{file}: %{line} ] %{message}");

    QString log = "";
    for(int i=0; i<argc; i++){
        log += " " + QString::fromLocal8Bit(argv[i]);
    }
    writeLog(log);

    MainWindow w;
    w.show();
    return a.exec();
}