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
};

#endif // PROPERTYDESKTOP_H
