#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QLineEdit>
#include <QFileSystemModel>
#include <QStandardItem>
#include "propertydesktop.h"
#include "ui_propertydesktop.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString BS(qint64 b);
    QLineEdit *LELocation;
    QFileSystemModel *model;
    QString path,source,pathIcon,pathDesktop,dir;
    QStandardItem *SI1,*SI2,*SI3,*SI4,*SI5,*SI6;
    QStandardItemModel *SIM;
    int cut;
    PropertyDesktop *dialogPD;
    QMenu *sortMenu;
    QAction *action_sortName,*action_sortSize,*action_sortType,*action_sortTime;

protected:
    void wheelEvent(QWheelEvent*);

private slots:
    void on_action_changelog_triggered();
    void on_action_about_triggered();
    void on_action_icon_triggered();
    void on_action_list_triggered();
    void on_action_back_triggered();
    void on_action_forward_triggered();
    void open(QModelIndex index);
    void info(QModelIndex index);
    void nav(QModelIndex index);
    void openL();
    void viewContextMenu(const QPoint &position);
    void viewContextMenuTV(const QPoint &position);
    void enterOpen();
};

#endif // MAINWINDOW_H
