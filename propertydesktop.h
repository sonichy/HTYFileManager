#ifndef PROPERTYDESKTOP_H
#define PROPERTYDESKTOP_H

#include <QDialog>

namespace Ui {
class PropertyDesktop;
}

class PropertyDesktop : public QDialog
{
    Q_OBJECT

public:
    explicit PropertyDesktop(QWidget *parent = 0);
    ~PropertyDesktop();
    Ui::PropertyDesktop *ui;
    QString filePath, iconPath;

private:
    void writeSettings(QString path, QString group, QString key, QString value);

private slots:
    void changeIcon();
    void changeExec();
    void openPath();
    void lineEditExecTextChanged(QString newpath);

//public slots:
    void saveDesktop();
};

#endif // PROPERTYDESKTOP_H
