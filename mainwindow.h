#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>

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
    void changeIcon();
    void saveDesktop();
    void desktopPath();
};

#endif // MAINWINDOW_H
