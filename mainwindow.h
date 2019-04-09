#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define LOCATION_OF_REAL_PATH Qt::UserRole + 1

#include "propertydesktop.h"
#include "ui_propertydesktop.h"
#include <QMainWindow>
#include <QModelIndex>
#include <QLineEdit>
#include <QFileInfoList>
#include <QListWidgetItem>
//#include <QFileSystemWatcher>

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
    QLineEdit *lineEditLocation, *lineEditSearch;
    QString path, source, pathIcon, pathDesktop, dir, pathSource, dirTrash, dirTrashInfo;
    int cut;
    PropertyDesktop *dialogPD;
    QMenu *sortMenu;
    QAction *action_sortName, *action_sortSize, *action_sortType, *action_sortTime;
    QModelIndexList modelIndexList;
    //QList<QListWidgetItem*> selected_files;
    QStringList SL_selected_files;
    void trashFiles();
    void deleteFiles();
    QString readSettings(QString path, QString group, QString key);
    void open(QString path);
    QFileInfoList list;
    bool delDir(QString dirpath);
    QScrollBar *verticalScrollBar;
    void iconPreview(int v);
    bool isPreviewFinish, isShowHidden;
    //QFileSystemWatcher *watcher;
    void readSettings();

protected:
    void wheelEvent(QWheelEvent*);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_action_changelog_triggered();
    void on_action_about_triggered();
    void on_action_icon_triggered();
    void on_action_list_triggered();
    void on_action_back_triggered();
    void on_action_forward_triggered();
    //void nav(QModelIndex index);
    void nav(QListWidgetItem *item);
    //void listWidgetClicked(QModelIndex index);
    void listWidgetDoubleClicked(QModelIndex index);
    void listWidgetItemClicked(QListWidgetItem *item);
    void listWidgetItemSelectionChanged();
    void tableWidgetDoubleClicked(QModelIndex index);
    void lineEditLocationReturnPressed();
    void customContextMenu(const QPoint &pos);
    //void viewContextMenuTV(const QPoint &pos);
    void enterOpen();
    void search();
    void trashDelete();
    void copy();
    void paste();
    void genList(QString spath);
    void verticalScrollBarValueChanged(int v);
    void switchHidden();
    void refresh();
    void rename();
    void listWidgetItemChanged(QListWidgetItem *LWI);

};

#endif // MAINWINDOW_H
