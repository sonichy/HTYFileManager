#include "propertydesktop.h"
#include "ui_propertydesktop.h"

PropertyDesktop::PropertyDesktop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertyDesktop)
{
    ui->setupUi(this);
    setFixedSize(400,300);
}

PropertyDesktop::~PropertyDesktop()
{
    delete ui;
}
