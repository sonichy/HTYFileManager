#ifndef FORM_DISK_H
#define FORM_DISK_H

#include <QWidget>

namespace Ui {
class Form_disk;
}

class Form_disk : public QWidget
{
    Q_OBJECT

public:
    explicit Form_disk(QWidget *parent = 0);
    ~Form_disk();
    Ui::Form_disk *ui;
    qint64 bytesFree, bytesTotal;
    QString mountPath, name;
    QByteArray device, fileSystemType;
    void init();

private:
    QString BS(qint64 b);

};

#endif // FORM_DISK_H