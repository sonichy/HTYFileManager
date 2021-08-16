#include "form_disk.h"
#include "ui_form_disk.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "iconpreview.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include <QLabel>
#include <QProcess>
#include <QDebug>
#include <QMimeDatabase>
#include <QPushButton>
#include <QWheelEvent>
#include <QScrollBar>
#include <QDateTime>
#include <QFileDialog>
#include <QShortcut>
#include <QFileIconProvider>
#include <QTextBrowser>
#include <QListWidgetItem>
#include <QStandardPaths>
#include <QTextCodec>
#include <QImageReader>
#include <QStorageInfo>
#include <QCheckBox>
#include <QDesktopServices>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    ui->setupUi(this);
    readSettings();
    isShowHidden = false;
    setStyleSheet("#listWidget { background-image:url(bg.jpg); }"
                  "QListWidget::item { background:transparent; }"
                  "QListWidget::item:selected { background: rgba(0,0,0,30); color: blue;} ");
    ui->tableWidget->setVisible(false);
    ui->action_icon->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    QActionGroup *AG = new QActionGroup(this);
    AG->addAction(ui->action_icon);
    AG->addAction(ui->action_list);

    dirTrash = QDir::homePath() + "/.local/share/Trash/files";
    dirTrashInfo = QDir::homePath() + "/.local/share/Trash/info";
    lineEditLocation = new QLineEdit(path, this);
    ui->mainToolBar->addWidget(lineEditLocation);
    connect(lineEditLocation, SIGNAL(returnPressed()), this, SLOT(lineEditLocationReturnPressed()));
    lineEdit_search = new QLineEdit("", this);
    lineEdit_search->setPlaceholderText("搜索");
    QAction *action_clear_lineEditSearch = new QAction;
    action_clear_lineEditSearch->setIcon(QIcon::fromTheme("edit-clear"));
    connect(action_clear_lineEditSearch, &QAction::triggered, [=]{
        lineEdit_search->clear();
        genList(path);
    });
    lineEdit_search->addAction(action_clear_lineEditSearch, QLineEdit::TrailingPosition);
    lineEdit_search->setFixedWidth(100);
    ui->mainToolBar->addWidget(lineEdit_search);
    //connect(lineEdit_search, SIGNAL(textChanged(QString)), this, SLOT(search()));
    connect(lineEdit_search, SIGNAL(returnPressed()), this, SLOT(search()));

    genHomePage();

    //connect(ui->listWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(listWidgetClicked(QModelIndex)));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listWidgetItemClicked(QListWidgetItem*)));
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(listWidgetItemSelectionChanged()));
    connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(listWidgetDoubleClicked(QModelIndex)));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenu(QPoint)));
    connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem)), this, SLOT(listWidgetItemChanged(QListWidgetItem)));

    connect(ui->listWidget_partition, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuPartition(QPoint)));

    verticalScrollBar = ui->listWidget->verticalScrollBar();
    connect(verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(verticalScrollBarValueChanged(int)));
    /*
    QShortcut *shortCutReturnListWidget = new QShortcut(QKeySequence(Qt::Key_Return), ui->listWidget);
    shortCutReturnListWidget->setObjectName("shortCutReturnListWidget");
    connect(shortCutReturnListWidget, SIGNAL(activated()),this, SLOT(enterOpen()));

    QShortcut *shortCutEnterListWidget = new QShortcut(QKeySequence(Qt::Key_Enter), ui->listWidget);
    shortCutEnterListWidget->setObjectName("shortCutEnterListWidget");
    connect(shortCutEnterListWidget, SIGNAL(activated()),this, SLOT(enterOpen()));

    QShortcut *shortCutReturnTableWidget = new QShortcut(QKeySequence(Qt::Key_Return), ui->tableWidget);
    shortCutReturnTableWidget->setObjectName("shortCutReturnTableWidget");
    connect(shortCutReturnTableWidget, SIGNAL(activated()),this, SLOT(enterOpen()));

    QShortcut *shortCutEnterTableWidget = new QShortcut(QKeySequence(Qt::Key_Enter), ui->tableWidget);
    shortCutEnterTableWidget->setObjectName("shortCutEnterTableWidget");
    connect(shortCutEnterTableWidget, SIGNAL(activated()),this, SLOT(enterOpen()));
*/
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), SIGNAL(activated()), this, SLOT(on_action_back_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Delete), this), SIGNAL(activated()), this, SLOT(trashDelete()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this), SIGNAL(activated()), this, SLOT(copy()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this), SIGNAL(activated()), this, SLOT(paste()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_H), this), SIGNAL(activated()), this, SLOT(switchHidden()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), this), SIGNAL(activated()), this, SLOT(zoomIn()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this), SIGNAL(activated()), this, SLOT(zoomOut()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0), this), SIGNAL(activated()), this, SLOT(zoom1()));
    connect(new QShortcut(QKeySequence(Qt::Key_F2), this), SIGNAL(activated()), this, SLOT(rename()));
    connect(new QShortcut(QKeySequence(Qt::Key_F5), this), SIGNAL(activated()), this, SLOT(refresh()));


    QStringList header;
    header << "名称" << "修改时间" << "大小" << "类型" << "歌名" << "歌手" << "专辑" << "年代" << "注释";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    connect(ui->tableWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(tableWidgetDoubleClicked(QModelIndex)));
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(viewContextMenuTV(QPoint)));

    dialogPD = new PropertyDesktop(this);
    connect(dialogPD, SIGNAL(accepted()), dialogPD, SLOT(saveDesktop()));

    sortMenu = new QMenu(this);
    action_sortName = new QAction(sortMenu);
    action_sortSize = new QAction(sortMenu);
    action_sortType = new QAction(sortMenu);
    action_sortTime = new QAction(sortMenu);
    action_sortName->setText("名称");
    action_sortSize->setText("大小");
    action_sortType->setText("类型");
    action_sortTime->setText("修改时间");
    action_sortName->setCheckable(true);
    action_sortSize->setCheckable(true);
    action_sortType->setCheckable(true);
    action_sortTime->setCheckable(true);
    action_sortName->setChecked(true);
    QActionGroup *actionGroupSort = new QActionGroup(this);
    sortMenu->addAction(actionGroupSort->addAction(action_sortName));
    sortMenu->addAction(actionGroupSort->addAction(action_sortSize));
    sortMenu->addAction(actionGroupSort->addAction(action_sortType));
    sortMenu->addAction(actionGroupSort->addAction(action_sortTime));

    QStringList args = QApplication::arguments();
    qDebug() << args;
    if (args.length() > 1) {
        path = args.at(1);
        if (path.startsWith("file://")) {
            QUrl url(args.at(1));
            path = url.toLocalFile();
        }
        qDebug() << path;
        if (!path.contains("/")) {
            QEventLoop loop;
            QProcess *process = new QProcess;
            process->start("pwd");
            connect(process, &QProcess::readyReadStandardOutput, [=, &loop](){
                QString pwd = process->readAllStandardOutput().trimmed();
                path = pwd + "/" +path;
                qDebug() << path;
                loop.quit();
            });
            loop.exec();
        }
        lineEditLocation->setText(path);
        lineEditLocationReturnPressed();
        ui->computerWidget->hide();
        if (ui->action_icon->isChecked())
            ui->listWidget->show();
        if (ui->action_list->isChecked())
            ui->tableWidget->show();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_changelog_triggered()
{
    QString s = "2.11\n2021-08\n恢复直接删除（不进回收站），修复没有写路径参数取不到路径无法删除的问题。\n\n2.10\n2020-06\n文件夹排在前面。\n实现图标模式排序。\n实现搜索（过滤）功能。\n修复盘符文字太宽挤开图标。\n\n2.9\n2020-04\n适配虚拟路径：trash:\\\\\\。\n\n2.8\n2020-03\n增加Git项目打包zip，tar.gz。\n\n2.7\n2019-09\n增加分区属性窗口。\n自定义分区控件，增加分区进度条。\n2019-07\n简化打开文件。\nresize自动滚动到选中文件的第一个。\n修复回收站文件还原没有刷新。\n使用Qt内部方法创建链接，识别链接并绘制链接角标。\n增加缩放快捷键。\n\n2.6\n2019-06\n增加：隐藏分区，取消隐藏分区功能。\n\n2.5\n2019-05\n修复：复制文件显示名而不是真实文件名导致粘贴失败的问题。\n修复：desktop属性窗口主题图标无法显示的问题。\n区分文件属性和文件夹属性。\n\n2.4\n2019-04\n导航增加系统盘。\n关闭时保存窗口位置和大小。\ndesktop属性窗口增加文件路径（只读）。\n粘贴文件后修改文件时间(>=5.10)。\n增加创建链接。\n\n2.3\n2018-12\n切换目录时设置导航栏。\n本地创建desktop失败，询问是否在桌面创建。\n修复显示文管主页时，地址栏打开路径不显示文件列表的问题。\ndesktop文件增加以管理员身份打开。\ndesktop无图标则显示默认图标。\n2018-11\n修复未知文件不显示图标问题。\n右键菜单移动文件后自动刷新当前目录。\n添加到深度文管目录打开方式列表。\n导航列表增加挂载分区，增加主页。\n\n2.2\n2018-07\n增加显示隐藏快捷键，刷新快捷键，增加图片打开方式。\n\n2.1\n2018-05\n列表模式可以显示MP3的ID3信息。\n\n2.0\n2018-04\n使用 QListWidget + Dir 遍历代替 QListView + QFileSystemModel，可以自定义文件图标。\n\n1.0\n2017-10\n增加文本文件打开方式菜单。\n文件列表回车快捷键与地址栏回车键冲突，引起有文件选中时地址栏回车无效，无文件选中时程序崩溃，暂时保留地址栏回车信号，取消程序的回车快捷键。\n粘贴有重名选择不覆盖将命名为副件XXX。\n2017-08\n多选复制粘贴删除成功，增加复制粘贴删除快捷键。\n增加搜索(过滤)。\n更新日志太长，由消息框改为文本框。\n2017-07\n增加视频文件打开方式，增加rmvb文件打开方式。\n增加背景图。\n增加压缩和解压缩菜单。\n2017-06\n属性窗体读取系统图标，增加回车键进入文件夹，增加退格键回到上层目录。\n属性窗体增加显示系统文件默认图标。\n从主窗体中分离属性窗体的代码。\n2017-05\n右键菜单增加【在终端中打开】。\n文件夹增加深度文管和Thunar打开方式。\n修复desktop已经存在，创建desktop会追加内容的BUG。\n单击文件在状态栏显示文件的MIME。\n2017-04\n图片右键菜单增加【设为壁纸】。\n文件右键菜单增加【移动到】、【复制到】。\n增加是否覆盖对话框。\ndesktop文件属性支持打开执行路径。\nQListView、QTableView实现排序。\n图标、列表按钮实现按下效果。\n实现删除文件到回收站，从回收站还原，优化回收站菜单。\n引号括起来，解决文件名含空格双击打不开的问题。\n增加列表模式右键菜单。\n增加管理员身份打开文件或文件夹。\n双击desktop文件，读取执行参数启动程序。\n增加修改desktop文件属性。\n解决QGridLayout单元格图标居中问题。\n增加读取desktop文件属性。\n增加新建文件夹，删除新建文件夹。\n程序右键增加创建快捷方式。\n图片的右键属性增加缩略图。\n2017-03\n增加左侧导航栏。\n增加右键菜单，增加复制、剪切、删除、属性功能。\n增加QTableView以列表形式显示，按钮切换图标、列表模式。\n增加后退功能。\n使用QListView以图标形式显示。";
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("更新历史");
    dialog->setFixedSize(400,300);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(s);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *pushButtonConfirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(pushButtonConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(pushButtonConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "海天鹰文件管理器 2.9\n一款基于 Qt5 的文件管理器。\n作者：海天鹰\nE-mail: sonichy@163.com\n主页：https://github.com/sonichy/HTYFileManager\n参考：\n右键菜单：http://windrocblog.sinaapp.com/?p=1016\n二级菜单：http://blog.csdn.net/u011417605/article/details/51219019\nQAction组群单选：http://qiusuoge.com/12287.html\nQListView添加项目：http://blog.csdn.net/u010142953/article/details/46694419\n修改文本：http://blog.csdn.net/caoshangpa/article/details/51775147\n获取系统文件图标：http://www.cnblogs.com/RainyBear/p/5223103.html");
    aboutMB.setIconPixmap(QPixmap(":/icon.png"));
    aboutMB.exec();
}

void MainWindow::nav(QListWidgetItem *item)
{
    path = item->data(LOCATION_OF_REAL_PATH).toString();
    //qDebug() << path;
    lineEditLocation->setText(path);
    if(path == "computer://"){
        ui->computerWidget->show();
        ui->listWidget->hide();
        ui->tableWidget->hide();
    }else{
        ui->computerWidget->hide();
        if(ui->action_icon->isChecked())ui->listWidget->show();
        if(ui->action_list->isChecked())ui->tableWidget->show();
        genList(path);
    }
}

void MainWindow::listWidgetDoubleClicked(QModelIndex index)
{
    Q_UNUSED(index);
    //qDebug() << "listWidgetDoubleClicked()" << index.data().toString();
    open(list.at(ui->listWidget->currentRow()).absoluteFilePath());
}

void MainWindow::tableWidgetDoubleClicked(QModelIndex index)
{
    Q_UNUSED(index);
    //qDebug() << "tableWidgetDoubleClicked()" << index.data().toString();
    open(list.at(ui->tableWidget->currentRow()).absoluteFilePath());
}

void MainWindow::open(QString newpath)
{
    qDebug() << "newpath:" << newpath;
    QFileInfo FI(newpath);
    if (FI.isDir() || newpath == "/" || newpath == "trash:///") {
        //lineEditLocation->setText(newpath);
        path = newpath;
        genList(path);
    } else {
        QProcess *proc = new QProcess;
        QString MIME = QMimeDatabase().mimeTypeForFile(newpath).name();
        qDebug() << MIME;
        if (MIME == "application/x-desktop") {
            QString sexec = readSettings(newpath, "Desktop Entry", "Exec");
            proc->setWorkingDirectory(readSettings(newpath, "Desktop Entry", "Path"));
            qDebug() << sexec;
            proc->start(sexec);
        } else if (MIME == "application/vnd.appimage") {
            proc->start(newpath);
        } else {
            QDesktopServices::openUrl(QUrl(newpath));
        }
    }
}

//void MainWindow::listWidgetClicked(QModelIndex index)
void MainWindow::listWidgetItemClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    //qDebug() << index.data().toString();
    qDebug() << "listWidgetItemClicked";
    QString filepath = list.at(ui->listWidget->currentRow()).absoluteFilePath();
    //QString MIME = QMimeDatabase().mimeTypeForFile(filepath).name();
    QString MIME = QMimeDatabase().mimeTypeForFile(filepath).comment();//MIME翻译
    ui->statusBar->showMessage("类型: " + MIME + ", 大小: " + BS(QFileInfo(filepath).size(),2) + ", 访问时间: " + QFileInfo(filepath).lastRead().toString("yyyy-MM-dd hh:mm:ss") + ", 修改时间: " + QFileInfo(filepath).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
}

void MainWindow::lineEditLocationReturnPressed()
{
    qDebug() << "lineEditLocationReturnPressed";
    QString newpath = lineEditLocation->text();
    QFileInfo FI(newpath);
    if (FI.isDir() || newpath == "trash:///") {
        path = newpath;
        ui->computerWidget->hide();
        ui->listWidget->show();
        genList(path);
    }
}

void MainWindow::on_action_back_triggered()
{
    if(path != "/"){
        int n = lineEditLocation->text().lastIndexOf("/");
        qDebug() << n;
        QString newpath = "";
        if (n>0) {
            newpath = lineEditLocation->text().left(n);
        } else {
            newpath = "/";
        }
        lineEditLocation->setText(newpath);
        path = newpath;
        genList(path);
    }
}

void MainWindow::on_action_forward_triggered()
{

}

void MainWindow::on_action_icon_triggered()
{
    if(path != "computer://"){
        ui->listWidget->setVisible(true);
        ui->tableWidget->setVisible(false);
    }
}

void MainWindow::on_action_list_triggered()
{
    if(path != "computer://"){
        ui->listWidget->setVisible(false);
        ui->tableWidget->setVisible(true);
    }
}

void MainWindow::customContextMenu(const QPoint &pos)
{
    QAction *action_copy, *action_cut, *action_rename, *action_trash, *action_emptyTrash, *action_delete, *action_restore, *action_paste, *action_newfolder, *action_sort, *action_property, *action_desktop, *action_gksu, *action_copyto, *action_moveto, *action_setWallpaper, *action_terminal, *action_zip, *action_unzip, *action_createLink;
    QModelIndex index = ui->listWidget->indexAt(pos);
    //QString filepath = path + "/" + index.data(Qt::DisplayRole).toString();
    QString filepath = "";
    if(index.isValid())
        filepath = list.at(ui->listWidget->currentRow()).absoluteFilePath();
    qDebug() << filepath;
    QString filename = QFileInfo(filepath).fileName();
    QString MIME = QMimeDatabase().mimeTypeForFile(filepath).name();
    qDebug() << MIME;
    QString filetype = MIME.left(MIME.indexOf("/"));

    QList<QAction*> actions;
    QAction *action_openwith = new QAction(this);
    action_openwith->setText("打开方式");
    actions.append(action_openwith);

    QMenu *openwithFileManager = new QMenu(this);
    QAction *DFM = new QAction(openwithFileManager);
    DFM->setText("深度文管");
    DFM->setIcon(QIcon::fromTheme("dde-file-manager"));
    openwithFileManager->addAction(DFM);
    QAction *Thunar = new QAction(openwithFileManager);
    Thunar->setText("Thunar");
    Thunar->setIcon(QIcon::fromTheme("Thunar"));
    openwithFileManager->addAction(Thunar);

    QMenu *openwithVideo = new QMenu(this);
    QAction *HTYMP = new QAction(openwithVideo);
    HTYMP->setText("海天鹰播放器");
    openwithVideo->addAction(HTYMP);

    QMenu *openwithText = new QMenu;
    QAction *deepin_editor = new QAction("深度编辑器", openwithText);
    deepin_editor->setIcon(QIcon::fromTheme("deepin-editor"));
    openwithText->addAction(deepin_editor);
    QAction *gedit = new QAction("gedit", openwithText);
    gedit->setIcon(QIcon::fromTheme("gedit"));
    openwithText->addAction(gedit);
    QAction *notepadqq = new QAction("notepadqq", openwithText);
    notepadqq->setIcon(QIcon::fromTheme("notepadqq"));
    openwithText->addAction(notepadqq);
    QAction *HTYEdit = new QAction("海天鹰编辑器", openwithText);
    openwithText->addAction(HTYEdit);

    QMenu *openwithImage = new QMenu;
    QAction *DIV = new QAction("深度看图", openwithImage);
    DIV->setIcon(QIcon::fromTheme("deepin-image-viewer"));
    openwithImage->addAction(DIV);
    QAction *HTYIV = new QAction("海天鹰看图", openwithImage);
    openwithImage->addAction(HTYIV);

    action_copy = new QAction(this);
    action_copy->setText("复制&C");   //不加&弹出菜单无法响应按键，加了后不需要setShortcut
    action_copy->setIcon(QIcon::fromTheme("edit-copy"));
    actions.append(action_copy);

    action_cut = new QAction(this);
    action_cut->setText("剪切&X");
    action_cut->setIcon(QIcon::fromTheme("edit-cut"));
    actions.append(action_cut);

    action_paste = new QAction(this);
    action_paste->setText("粘贴&V");
    action_paste->setIcon(QIcon::fromTheme("edit-paste"));
    actions.append(action_paste);

    action_rename = new QAction(this);
    action_rename->setText("重命名&M");
    actions.append(action_rename);

    action_moveto = new QAction(this);
    action_moveto->setText("移动到...");
    actions.append(action_moveto);

    action_copyto = new QAction(this);
    action_copyto->setText("复制到...");
    actions.append(action_copyto);

    action_zip = new QAction(this);
    action_zip->setText("压缩");
    action_zip->setIcon(QIcon::fromTheme("file-roller"));
    actions.append(action_zip);

    action_unzip = new QAction(this);
    action_unzip->setText("解压到当前目录");
    action_unzip->setIcon(QIcon::fromTheme("file-roller"));
    actions.append(action_unzip);

    QAction *action_git_tar_gz = new QAction(this);
    action_git_tar_gz->setText("Git打包tar.gz");
    action_git_tar_gz->setIcon(QIcon::fromTheme("file-roller"));
    actions.append(action_git_tar_gz);

    QAction *action_git_zip = new QAction(this);
    action_git_zip->setText("Git打包zip");
    action_git_zip->setIcon(QIcon::fromTheme("file-roller"));
    actions.append(action_git_zip);

    action_trash = new QAction(this);
    action_trash->setText("移至回收站");
    action_trash->setIcon(QIcon::fromTheme("user-trash-full"));
    actions.append(action_trash);

    action_emptyTrash = new QAction(this);
    action_emptyTrash->setText("清空回收站");
    action_emptyTrash->setIcon(QIcon::fromTheme("user-trash"));
    actions.append(action_emptyTrash);

    action_delete = new QAction(this);
    action_delete->setText("删除");
    action_delete->setIcon(QIcon::fromTheme("edit-delete"));
    actions.append(action_delete);

    action_restore = new QAction(this);
    action_restore->setText("还原");
    actions.append(action_restore);

    action_newfolder = new QAction(this);
    action_newfolder->setText("新建文件夹");
    action_newfolder->setIcon(QIcon::fromTheme("folder-new"));
    actions.append(action_newfolder);

    action_sort = new QAction(this);
    action_sort->setText("排序方式");
    actions.append(action_sort);
    action_sort->setMenu(sortMenu);

    action_setWallpaper = new QAction(this);
    action_setWallpaper->setText("设为壁纸");
    action_setWallpaper->setIcon(QIcon::fromTheme("preferences-desktop-wallpaper"));
    actions.append(action_setWallpaper);

    action_property = new QAction(this);
    action_property->setText("属性&R");
    action_property->setIcon(QIcon::fromTheme("document-properties"));
    actions.append(action_property);

    action_terminal = new QAction(this);
    action_terminal->setText("在终端中打开&T");
    action_terminal->setIcon(QIcon::fromTheme("utilities-terminal"));
    actions.append(action_terminal);

    action_desktop = new QAction(this);
    action_desktop->setText("创建快捷方式");
    actions.append(action_desktop);

    action_gksu = new QAction(this);
    action_gksu->setText("以管理员身份打开");
    actions.append(action_gksu);

    action_createLink = new QAction(this);
    action_createLink->setText("创建链接");
    action_createLink->setIcon(QIcon::fromTheme("emblem-symbolic-link"));
    actions.append(action_createLink);

    if (filetype == "video" || filetype == "audio" || MIME == "application/vnd.rn-realmedia") {
        action_openwith->setMenu(openwithVideo);
    }

    if (filetype == "text" || MIME == "application/javascript" || MIME == "application/x-desktop") {
        action_openwith->setMenu(openwithText);
    }

    if (MIME == "inode/directory") {
        action_openwith->setMenu(openwithFileManager);
    } else{
        action_zip->setVisible(false);
    }

    if (MIME != "application/zip") {
        action_unzip->setVisible(true);
    }

    if (MIME != "application/x-executable" && MIME != "application/x-shellscript" && MIME != "application/x-ms-dos-executable" && MIME !="application/x-sharedlib" && MIME != "application/vnd.appimage") action_desktop->setVisible(false);

    if (filetype == "image"){
        action_openwith->setMenu(openwithImage);
    }else{
        action_setWallpaper->setVisible(false);
    }

    qDebug() << path << dirTrash;
    if (path == dirTrash) {
        action_openwith->setVisible(false);
        action_paste->setVisible(false);
        action_moveto->setVisible(false);
        action_copyto->setVisible(false);
        action_unzip->setVisible(false);
        action_trash->setVisible(false);
        action_gksu->setVisible(false);
        action_rename->setVisible(false);
        action_newfolder->setVisible(false);
        action_terminal->setVisible(false);
        action_setWallpaper->setVisible(false);
        action_createLink->setVisible(false);
        if (index.isValid()) action_emptyTrash->setVisible(false);
    } else {
        //action_delete->setVisible(false);
        action_restore->setVisible(false);
        action_emptyTrash->setVisible(false);
    }
    if (index.isValid()) {
        action_newfolder->setVisible(false);
        action_sort->setVisible(false);
        action_git_tar_gz->setVisible(false);
        action_git_zip->setVisible(false);
    } else {
        qDebug() << "viewContextMenu: index is not valid";
        action_copy->setVisible(false);
        action_cut->setVisible(false);
        action_rename->setVisible(false);
        action_trash->setVisible(false);
        action_delete->setVisible(false);
        action_gksu->setVisible(false);
        action_restore->setVisible(false);
        action_createLink->setVisible(false);
        QDir dir_git(path + "/.git");
        if(!dir_git.exists()){
            action_git_tar_gz->setVisible(false);
            action_git_zip->setVisible(false);
        }
    }
    if (QFileInfo(filepath).isFile()) action_terminal->setVisible(false);
    if (QFileInfo(filepath).isSymLink()) action_createLink->setVisible(false);

    QAction *result_action = QMenu::exec(actions, ui->listWidget->mapToGlobal(pos));

    if (result_action == DFM) {
        QProcess *proc = new QProcess;
        QString cmd = "dde-file-manager \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if (result_action == Thunar) {
        QProcess *proc = new QProcess;
        QString cmd = "thunar \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if (result_action == deepin_editor) {
        QProcess *proc = new QProcess;
        QString cmd = "deepin-editor \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if (result_action == gedit) {
        QProcess *proc = new QProcess;
        QString cmd = "gedit \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if (result_action == notepadqq) {
        QProcess *proc = new QProcess;
        QString cmd = "notepadqq \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if (result_action == HTYEdit) {
        QProcess *proc = new QProcess;
        QString cmd = "/media/sonichy/job/HY/Linux/Qt/HTYEdit/HTYEdit \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if (result_action == DIV) {
        QProcess *proc = new QProcess;
        QString cmd = "deepin-image-viewer \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if (result_action == HTYIV) {
        QProcess *proc = new QProcess;
        QString cmd = "/media/sonichy/job/HY/Linux/Qt/HTYIV/HTYIV \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if (result_action == action_copy) {
        copy();
        return;
    }

    if (result_action == action_cut) {
        cut = 1;
        copy();
        return;
    }

    if(result_action == action_paste){
        paste();
        /*
        qDebug() << "paste";
        for(int i=0; i<SL_selected_files.length(); i++){
            QString source = SL_selected_files.at(i);
            QString newName = path + "/" + QFileInfo(SL_selected_files.at(i)).fileName();
            qDebug() << "paste" << source << newName;
            if(!QFile::copy(source, newName)){
                QMessageBox::StandardButton SB = QMessageBox::warning(NULL, "覆盖", "是否覆盖 " + newName + " ?", QMessageBox::Yes | QMessageBox::No |QMessageBox::Cancel, QMessageBox::Yes);
                if(SB == QMessageBox::Yes){
                    if(!QFile::remove(newName)){
                        QMessageBox::critical(NULL, "错误", "无法覆盖新文件 " + newName);
                    }
                    if(!QFile::copy(source, newName)){
                        QMessageBox::critical(NULL, "错误", "粘贴失败！");
                    }
                }else if(SB == QMessageBox::No){
                    newName =  path + "/副本-" + QFileInfo(SL_selected_files.at(i)).fileName();
                    qDebug() << "副本" << source << newName;
                    if(!QFile::copy(source, newName)){
                        QMessageBox::critical(NULL, "错误", "粘贴失败！");
                    }
                }
            }else{
                // 修改粘贴文件时间为原文件时间
                QFile file(newName);
                qDebug() << "修改粘贴文件时间为原文件时间" << file.setFileTime(QFileInfo(source).lastModified(), QFileDevice::FileModificationTime);
                if(cut){
                    if(!QFile::remove(source)){
                        QMessageBox::critical(NULL, "错误", "无法删除剪切的源文件 " + source, QMessageBox::Ok);
                    }
                }
                cut=0;
            }
        }
        */
        return;
    }

    if (result_action == action_trash) {
        trashFiles();
        return;
    }

    if (result_action == action_emptyTrash) {
        QMessageBox msgBox(QMessageBox::Warning, "清空回收站", "你确定要清空回收站？", QMessageBox::Yes | QMessageBox::No);
        msgBox.setButtonText(QMessageBox::Yes, "是");
        msgBox.setButtonText(QMessageBox::No, "否");
        int ret = msgBox.exec();
        //QMessageBox::StandardButton SB = QMessageBox::question(NULL, "清空回收站", "你确定要清空回收站？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes) {
            ui->statusBar->showMessage("清空回收站...");
            delDir(dirTrash);
            genList(dirTrash);
            ui->statusBar->showMessage("");
        }
        return;
    }

    if (result_action == action_delete) {
        deleteFiles();
        return;
    }

    if (result_action == action_restore) {
        qDebug() << "restore" << filepath;
        QString spath = "";
        QString pathinfo = QDir::homePath()+"/.local/share/Trash/info/" + QFileInfo(filepath).fileName() + ".trashinfo";
        QFile file(pathinfo);
        file.open(QIODevice::ReadOnly);
        while (!file.atEnd()) {
            QString sl = file.readLine().replace("\n","");
            //qDebug() << sl;
            if(sl.left(sl.indexOf("=")).toLower() == "path"){
                spath = sl.mid(sl.indexOf("=")+1);
                break;
            }
        }
        qDebug() << spath;
        if (QFile::copy(filepath, spath)) {
            QFile::remove(filepath);
            QFile::remove(QDir::homePath() + "/.local/share/Trash/info/" + QFileInfo(filepath).fileName() + ".trashinfo");
            genList(path);
        }
        return;
    }

    if (result_action == action_newfolder) {
        QDir *dir = new QDir;
        if(!dir->mkdir(path + "/" + "新建文件夹")){
            QMessageBox::warning(this, "创建文件夹", "文件夹已经存在！");
        }
        return;
    }

    if (result_action == action_rename) {
        QDialog *dialog = new QDialog(this);
        dialog->setWindowFlags(Qt::Tool);
        dialog->setFixedSize(200,100);
        dialog->setWindowTitle("重命名");
        QVBoxLayout *vbox = new QVBoxLayout;
        QLineEdit *lineEdit = new QLineEdit;
        lineEdit->setText(filename);
        vbox->addWidget(lineEdit);
        QHBoxLayout *hbox = new QHBoxLayout;
        QPushButton *pushButtonConfirm = new QPushButton("确定");
        QPushButton *pushButtonCancel = new QPushButton("取消");
        hbox->addWidget(pushButtonConfirm);
        hbox->addWidget(pushButtonCancel);
        vbox->addLayout(hbox);
        dialog->setLayout(vbox);
        connect(pushButtonConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(pushButtonCancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        if(dialog->exec() == QDialog::Accepted){
            QString newName = QFileInfo(filepath).absolutePath() + "/" + lineEdit->text();
            qDebug() << "rename" << filepath << newName;
            if (QFile::rename(filepath, newName)) {
                genList(QFileInfo(filepath).absolutePath());
            }else{
                QMessageBox::critical(NULL, "错误", "无法重命名文件，该文件已存在！", QMessageBox::Ok);
            }
        }
        dialog->close();
        return;
    }

    if (result_action == action_property) {
        if (index.isValid()) {
            if (MIME == "application/x-desktop") {
                QString sExec = readSettings(filepath, "Desktop Entry", "Exec");
                QString sName = readSettings(filepath, "Desktop Entry", "Name");
                QString sIcon = readSettings(filepath, "Desktop Entry", "Icon");
                QString sPath = readSettings(filepath, "Desktop Entry", "Path");
                QString sComment = readSettings(filepath, "Desktop Entry", "Comment");
                QString sCategories = readSettings(filepath, "Desktop Entry", "Categories");
                //dialogPD->ui->lineEditPathDesktop->setText(filepath);
                dialogPD->filePath = filepath;
                if(sIcon.contains("/")){
                    dialogPD->ui->pushButton_icon->setIcon(QIcon(sIcon));
                }else{
                    dialogPD->ui->pushButton_icon->setIcon(QIcon::fromTheme(sIcon));
                }
                //dialogPD->ui->lineEditIcon->setText(sicon);
                dialogPD->iconPath = sIcon;
                dialogPD->ui->lineEdit_filepath->setText(filepath);
                dialogPD->ui->lineEdit_name->setText(sName);
                dialogPD->ui->lineEdit_name->setCursorPosition(0);
                dialogPD->ui->lineEdit_exec->setText(sExec);
                dialogPD->ui->lineEdit_exec->setCursorPosition(0);
                dialogPD->ui->lineEdit_path->setText(sPath);
                dialogPD->ui->lineEdit_path->setCursorPosition(0);
                dialogPD->ui->lineEdit_comment->setText(sComment);
                dialogPD->ui->lineEdit_comment->setCursorPosition(0);
                dialogPD->ui->lineEdit_categories->setText(sCategories);
                dialogPD->ui->lineEdit_categories->setCursorPosition(0);
                dialogPD->show();
            } else {
                qDebug() << "property" << filepath;
                QString path1 = "";
                if (QFileInfo(filepath).isSymLink()) {
                    path1 = "链接路径：\t" + QFileInfo(filepath).symLinkTarget();
                } else {
                    path1 = "路径：\t" + path;
                }
                QMessageBox MBox(QMessageBox::NoIcon, "属性", path1 + "\n文件名：\t" + QFileInfo(filepath).fileName() + "\n大小：\t" + BS(QFileInfo(filepath).size(),2) + "\n类型：\t" + QMimeDatabase().mimeTypeForFile(filepath).name() + "\n访问时间：\t" + QFileInfo(filepath).lastRead().toString("yyyy-MM-dd hh:mm:ss") + "\n修改时间：\t" + QFileInfo(filepath).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
                QIcon icon = ui->listWidget->currentItem()->icon();
                MBox.setIconPixmap(icon.pixmap(QSize(150,150)));
                MBox.setWindowFlags(Qt::Tool);
                MBox.exec();
            }
        }else{
            QMessageBox MBox(QMessageBox::NoIcon, "文件夹属性", "路径：\t" + path + "\n大小：\t" + BS(QFileInfo(path).size(),2) + "\n访问时间：\t" + QFileInfo(path).lastRead().toString("yyyy-MM-dd hh:mm:ss") + "\n修改时间：\t" + QFileInfo(path).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            MBox.setIconPixmap(QIcon::fromTheme("folder").pixmap(QSize(150,150)));
            MBox.exec();
        }
        return;
    }

    if (result_action == action_desktop) {
        qDebug() << "Create desktop file";
        QString iconName = "icon.png";
        if(MIME == "application/vnd.appimage") iconName = QFileInfo(filepath).baseName() + ".png";
        QString sExec;
        if(MIME=="application/x-ms-dos-executable"){
            sExec = "deepin-wine " + filepath;
        }else{
            sExec = filepath;
        }
        QFile file(QFileInfo(filepath).absolutePath() + "/" + QFileInfo(filepath).baseName() + ".desktop");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QString str = "[Desktop Entry]\nName=" + QFileInfo(filepath).baseName() + "\nComment=\nExec=" + sExec + "\nIcon=" + QFileInfo(filepath).absolutePath() + "/" + iconName + "\nPath=" + QFileInfo(filepath).absolutePath() + "\nTerminal=false\nType=Application\nMimeType=\nCategories=";
            qDebug() << str;
            QTextStream in(&file);
            in << str;
            file.close();
            genList(path);
        }else{
            //QMessageBox::warning(this,"错误","不能创建 " + filepath + ".desktop", QMessageBox::Yes);
            QMessageBox MB(QMessageBox::Question, "错误", "不能创建 " + filepath + ".desktop，是否在桌面创建？", QMessageBox::Yes | QMessageBox::No, NULL);
            if (MB.exec() ==  QMessageBox::Yes) {
                file.setFileName(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + QFileInfo(filepath).baseName() + ".desktop");
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    iconName = QFileInfo(filepath).baseName();
                    QString str = "[Desktop Entry]\nName=" + QFileInfo(filepath).baseName() + "\nComment=\nExec=" + sExec + "\nIcon=" + iconName + "\nPath=" + QFileInfo(filepath).absolutePath() + "\nTerminal=false\nType=Application\nMimeType=\nCategories=";
                    qDebug() << str;
                    QTextStream in(&file);
                    in << str;
                    file.close();
                    genList(path);
                }
            }
        }
        return;
    }

    if (result_action == action_gksu) {
        qDebug() << "gksu";
        QProcess *proc = new QProcess;
        if(filetype == "text" || MIME == "application/x-desktop"){
            proc->start("gksu gedit " + filepath);
        }
        if(MIME == "inode/directory"){
            proc->start("gksu " + QDir::currentPath() + "/HTYFileManager " + filepath);
        }
    }

    if(result_action == action_terminal){
        QProcess *proc = new QProcess;
        QString cmd = "";
        if (index.isValid()) {
            cmd = "deepin-terminal -w " + filepath;
        } else {
            cmd = "deepin-terminal -w " + path;
        }
        qDebug() << cmd;
        proc->start(cmd);
    }

    if (result_action == action_sortName) {
        qDebug() << "sort by name";
        sortFlags = QDir::Name | QDir::DirsFirst;
        genList(path);
    }

    if (result_action == action_sortSize) {
        qDebug() << "sort by size";
        sortFlags = QDir::Size | QDir::DirsFirst;
        genList(path);
    }

    if (result_action == action_sortType) {
        qDebug() << "sort by type";
        sortFlags = QDir::Type | QDir::DirsFirst;
        genList(path);
    }

    if(result_action == action_sortTime){
        qDebug() << "sort by time";
        sortFlags = QDir::Time | QDir::DirsFirst;
        genList(path);
    }

    if (result_action == action_copyto) {
        if (dir == "") dir = "/home";
        dir = QFileDialog::getExistingDirectory(this, "选择路径", dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (dir != "") {
            QString newName = dir + "/" + QFileInfo(filepath).fileName();
            qDebug() << "copyto:" << filepath << newName;
            if (!QFile::copy(filepath, newName)) {
                QMessageBox::StandardButton SB = QMessageBox::warning(NULL, "覆盖", "是否覆盖 "+newName+" ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (SB == QMessageBox::Yes) {
                    if (!QFile::remove(newName)) {
                        QMessageBox::critical(NULL, "错误", "无法覆盖新文件 "+newName);
                    } else {
                        qDebug() << "copyto" << newName;
                        refresh();
                    }
                    if (!QFile::copy(filepath, newName)) {
                        QMessageBox::critical(NULL, "错误", "粘贴失败！");
                    } else {
                        qDebug() << "copy" << filepath << newName;
                    }
                }
            }else{
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
                QFile file(newName);
                file.open(QIODevice::ReadOnly);
                qDebug() << "修改粘贴文件时间为原文件时间" << file.setFileTime(QFileInfo(filepath).lastModified(), QFileDevice::FileModificationTime);
                file.close();
#endif
            }
        }
        return;
    }

    if (result_action == action_moveto) {
        if (dir == "") dir = "/home";
        dir = QFileDialog::getExistingDirectory(this, "选择路径", dir, QFileDialog::ShowDirsOnly |QFileDialog::DontResolveSymlinks);
        if (dir != "") {
            QList<QListWidgetItem*> selected_files = ui->listWidget->selectedItems();
            for (int i=0; i<selected_files.length(); i++) {
                QString fp = path + "/" + selected_files.at(i)->text();
                QString newName = dir + "/" + selected_files.at(i)->text();
                qDebug() << "moveto:" << fp << newName;
                if (QFile::copy(fp, newName)) {
                    qDebug() << "copy" << fp << newName;
                    if (QFile::remove(fp)) {
                        qDebug() << "remove" << fp;
                    }else{
                        QMessageBox::critical(NULL, "错误", "无法删除源文件 " + fp);
                    }
                }else{
                    QMessageBox::StandardButton SB = QMessageBox::warning(NULL, "覆盖", "是否覆盖 " + newName + " ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    if (SB == QMessageBox::Yes) {
                        if (QFile::remove(newName)) {
                            qDebug() << "remove" << newName;
                            refresh();
                        } else {
                            QMessageBox::critical(NULL, "错误", "无法覆盖新文件 " + newName);
                        }
                        if (!QFile::copy(fp, newName)) {
                            QMessageBox::critical(NULL, "错误", "粘贴失败！");
                        } else {
                            qDebug() << "copy" << fp << newName;
                            if (!QFile::remove(fp)) {
                                QMessageBox::critical(NULL, "错误", "无法删除源文件 " + fp);
                            }else{
                                qDebug() << "remove" << fp;
                            }
                        }
                    }
                }
            }
        }
        refresh();
        return;
    }

    if (result_action == action_setWallpaper) {
        QString cmd = "gsettings set org.gnome.desktop.background picture-uri \"file://" + filepath + "\"";
        qDebug() << "setWallpaper:" << cmd;
        QProcess *proc = new QProcess;
        proc->start(cmd);
        return;
    }

    if (result_action == action_zip) {
        QString cmd = "zip -rj " + filepath + ".zip " + filepath;
        qDebug() << cmd;
        QProcess *proc = new QProcess;
        proc->start(cmd);
        return;
    }

    if (result_action == action_unzip) {
        QString cmd = "unzip -d " + QFileInfo(filepath).absolutePath() + "/" + QFileInfo(filepath).baseName() + " " + filepath;
        qDebug() << cmd;
        QProcess *proc = new QProcess;
        proc->start(cmd);
        return;
    }

    if (result_action == action_git_tar_gz) {
        QProcess *process1 = new QProcess;
        QProcess *process2 = new QProcess;
        process1->setStandardOutputProcess(process2);
        process1->setWorkingDirectory(path);
        process2->setWorkingDirectory(path);
        connect(process1, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
        connect(process1, SIGNAL(readyReadStandardError()), this, SLOT(processError()));
        connect(process2, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
        connect(process2, SIGNAL(readyReadStandardError()), this, SLOT(processError()));
        process1->start("git ls-files");
        process2->start("xargs tar -czvf " + QFileInfo(path).fileName() + ".tar.gz");
        if(process2->waitForFinished())
            genList(path);
        return;
    }

    if (result_action == action_git_zip) {
        QProcess *process1 = new QProcess;
        process1->setWorkingDirectory(path);
        connect(process1, SIGNAL(readyReadStandardError()), this, SLOT(processError()));
        process1->start("git ls-files");
        process1->waitForFinished();
        QString s = QString(process1->readAllStandardOutput()).replace("\n"," ");
        qDebug() << s;
        s = "zip " + QFileInfo(path).fileName() + ".zip -xi " + s;
        qDebug() << s;
        connect(process1, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
        process1->start(s);
        if(process1->waitForFinished())
            genList(path);
        return;
    }

    if (result_action == HTYMP) {
        QString desktopPath = QDir::homePath() + "/.local/share/applications/HTYMP.desktop";
        QString sexec = readSettings(desktopPath, "Desktop Entry", "Exec");
        QProcess *proc = new QProcess;
        proc->start(sexec);
        return;
    }

    if (result_action == action_createLink) {
        QString dest = QFileInfo(filepath).absolutePath() + "/快捷方式-" + QFileInfo(filepath).fileName();
        QFile file(filepath);
        if(file.link(dest))
            genList(QFileInfo(filepath).absolutePath());
        return;
    }

    foreach (QAction* action, actions) {
        action->deleteLater();
    }
}

/*
void MainWindow::viewContextMenuTV(const QPoint &position)
{
    QAction *action_copy,*action_cut,*action_rename,*action_trash,*action_delete,*action_paste,*action_newdir,*action_property,*action_desktop,*action_gksu;
    QModelIndex index = ui->tableWidget->indexAt(position);
    //QString filepath = index.data(QFileSystemModel::FilePathRole).toString();
    QString filename = QFileInfo(filepath).fileName();
    QString MIME = QMimeDatabase().mimeTypeForFile(filepath).name();
    qDebug() << MIME;
    QString filetype = MIME.left(MIME.indexOf("/"));

    QList<QAction *> actions;
    action_copy = new QAction(this);
    action_copy->setText("复制");
    actions.append(action_copy);

    action_cut = new QAction(this);
    action_cut->setText("剪切");
    actions.append(action_cut);

    action_rename = new QAction(this);
    action_rename->setText("重命名");
    actions.append(action_rename);

    action_delete = new QAction(this);
    action_delete->setText("删除");
    actions.append(action_delete);

    action_paste = new QAction(this);
    action_paste->setText("粘贴");
    actions.append(action_paste);

    action_newdir = new QAction(this);
    action_newdir->setText("新建文件夹");
    actions.append(action_newdir);

    action_property = new QAction(this);
    action_property->setText("属性");
    actions.append(action_property);

    action_desktop = new QAction(this);
    action_desktop->setText("创建快捷方式");
    actions.append(action_desktop);

    action_gksu = new QAction(this);
    action_gksu->setText("以管理员身份打开");
    actions.append(action_gksu);

    if(MIME != "application/x-executable") action_desktop->setVisible(false);

    if(!index.isValid()){
        qDebug() << "viewContextMenu: index is not valid";
        action_copy->setVisible(false);
        action_cut->setVisible(false);
        action_rename->setVisible(false);
        action_delete->setVisible(false);
        action_gksu->setVisible(false);
    }

    QAction *result_action = QMenu::exec(actions, ui->tableView->mapToGlobal(position));

    if(result_action == action_copy){
        source=filepath;
        qDebug() << "copy" << source;
        return;
    }

    if(result_action == action_cut){
        qDebug() << "cut" << source;
        source=filepath;
        cut=1;
        return;
    }

    if(result_action == action_paste){
        QString newName = path + "/" + QFileInfo(source).fileName();
        qDebug() << "paste" << source << newName;
        if(!QFile::copy(source, newName)){
            QMessageBox::critical(NULL, "错误", "粘贴失败！", QMessageBox::Ok);
        }else{
            if(cut){
                if(!QFile::remove(source)){
                    QMessageBox::critical(NULL, "错误", "无法删除剪切的源文件 "+source, QMessageBox::Ok);
                }
            }
            cut=0;
        }
        return;
    }

    if(result_action == action_delete){
        qDebug() << "delete" << filepath;
        if(MIME == "inode/directory"){
            QDir *dir = new QDir;
            if(!dir->rmdir(filepath)){
                QMessageBox::critical(this,"错误","无法删除文件夹 "+filepath);
            }
        }else{
            if(!QFile::remove(filepath)){
                QMessageBox::critical(NULL, "错误", "无法删除文件 "+filepath);
            }
        }
        return;
    }

    if(result_action == action_newdir){
        QDir *dir = new QDir;
        if(!dir->mkdir(path+"/"+"新建文件夹")){
            QMessageBox::warning(this,"创建文件夹","文件夹已经存在！");
        }
        return;
    }

    if(result_action == action_rename){
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle("重命名");
        QVBoxLayout *vbox = new QVBoxLayout;
        QLineEdit *lineEdit = new QLineEdit;
        lineEdit->setText(filename);
        vbox->addWidget(lineEdit);
        QHBoxLayout *hbox = new QHBoxLayout;
        QPushButton *btnConfirm = new QPushButton("确定");
        QPushButton *btnCancel = new QPushButton("取消");
        hbox->addWidget(btnConfirm);
        hbox->addWidget(btnCancel);
        vbox->addLayout(hbox);
        dialog->setLayout(vbox);
        connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(btnCancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        if(dialog->exec() == QDialog::Accepted){
            QString newName = QFileInfo(filepath).absolutePath() + "/" + lineEdit->text();
            qDebug() << "rename" << filepath << newName;
            if(!QFile::rename(filepath, newName)){
                QMessageBox::critical(NULL, "错误", "无法重命名文件，该文件已存在！", QMessageBox::Ok);
            }
        }
        dialog->close();
        return;
    }

    if(result_action == action_property){
        if(MIME == "application/x-desktop"){
            pathDesktop = filepath;
            QString sname = "", sexec = "", spath = "", scomment = "";
            QFile file(filepath);
            file.open(QIODevice::ReadOnly);
            while(!file.atEnd()){
                QString sl = file.readLine().replace("\n","");
                qDebug() << sl;
                if(sl.left(sl.indexOf("=")).toLower() == "name"){
                    sname = sl.mid(sl.indexOf("=") + 1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower() == "exec"){
                    sexec = sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower() == "icon"){
                    pathIcon = sl.mid(sl.indexOf("=") + 1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower() == "path"){
                    spath=sl.mid(sl.indexOf("=") + 1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower() == "comment"){
                    scomment = sl.mid(sl.indexOf("=") + 1);
                    continue;
                }
            }
            dialogPD->ui->pushButtonIcon->setIcon(QIcon(pathIcon));
            dialogPD->ui->lineEditIcon->setText(pathIcon);
            dialogPD->ui->lineEditName->setText(sname);
            dialogPD->ui->lineEditName->setCursorPosition(0);
            dialogPD->ui->lineEditExec->setText(sexec);
            dialogPD->ui->lineEditExec->setCursorPosition(0);
            dialogPD->ui->lineEditPath->setText(spath);
            dialogPD->ui->lineEditPath->setCursorPosition(0);
            dialogPD->ui->lineEditComment->setText(scomment);
            dialogPD->ui->lineEditComment->setCursorPosition(0);
            dialogPD->show();
        }else{
            qDebug() << "property" << filepath;
            QMessageBox MBox(QMessageBox::NoIcon, "属性", "文件名：\t" + QFileInfo(filepath).fileName() + "\n大小：\t" + BS(QFileInfo(filepath).size(),2) + "\n类型：\t" + QMimeDatabase().mimeTypeForFile(filepath).name() + "\n访问时间：\t" + QFileInfo(filepath).lastRead().toString("yyyy-MM-dd hh:mm:ss") + "\n修改时间：\t" + QFileInfo(filepath).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            if(filetype == "image"){
                QSize iconSize(200,200);
                MBox.setIconPixmap(QPixmap(filepath).scaled(iconSize, Qt::KeepAspectRatio));
            }
            MBox.exec();
        }
        return;
    }

    if(result_action == action_desktop) {
        qDebug() << "Create desktop file";
        QString str = "[Desktop Entry]\nName=" + QFileInfo(filepath).fileName() + "\nComment=\nExec=" + filepath + "\nIcon=" + QFileInfo(filepath).absolutePath() + "/icon.png\nPath=" + QFileInfo(filepath).absolutePath() + "\nTerminal=false\nType=Application\nMimeType=\nCategories=";
        QFile file(QFileInfo(filepath).absolutePath() + "/" + QFileInfo(filepath).fileName() + ".desktop");
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
            QMessageBox::warning(this,"错误","不能创建 "+filepath+".desktop",QMessageBox::Yes);
        }
        QTextStream in(&file);
        in << str;
        file.close();
        return;
    }

    if(result_action == action_gksu) {
        QProcess *proc = new QProcess;
        if(filetype == "text"){
            proc->start("gksu gedit " + filepath);
        }
        if(MIME == "inode/directory"){
            proc->start("gksu " + QDir::currentPath() + "/HTYFileManager " + filepath);
        }
    }

    foreach(QAction* action, actions) {
        action->deleteLater();
    }
}
*/

void MainWindow::customContextMenuPartition(const QPoint &pos)
{
    QModelIndex index = ui->listWidget_partition->indexAt(pos);
    QListWidgetItem *LWI = ui->listWidget_partition->itemAt(pos);
    QAction *action_hide, *action_property;
    QList<QAction *> actions;
    action_hide = new QAction(this);
    action_hide->setText("隐藏");
    actions.append(action_hide);

    action_property = new QAction(this);
    action_property->setText("属性&R");
    actions.append(action_property);

    if (index.isValid()) {

    }else{
        action_hide->setVisible(false);
    }

    //在菜单中显示快捷键(QT5.7已经显示，不需要)
#if (QT_VERSION >= QT_VERSION_CHECK(5,10,0))
    foreach(QAction *action, actions){
        action->setShortcutVisibleInContextMenu(true);
    }
#endif

    QAction *result_action = QMenu::exec(actions, ui->listWidget_partition->mapToGlobal(pos));

    if (result_action == action_hide) {
        QString name = QFileInfo(LWI->data(LOCATION_OF_REAL_PATH).toString()).fileName();
        qDebug() << name;
        if(name != ""){
            QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
            if(settings.value("partition_hide") == ""){
                settings.setValue("partition_hide", name);
            }else{
                settings.setValue("partition_hide", settings.value("partition_hide").toString() + ";" + name);
            }
            genHomePage();
        }
        return;
    }

    if (result_action == action_property) {
        if (index.isValid()) {
            Form_disk *form_disk = (Form_disk*)(ui->listWidget_partition->itemWidget(ui->listWidget_partition->currentItem()));
            QDialog *dialog = new QDialog;
            dialog->setWindowTitle("属性");
            dialog->setFixedSize(300,250);
            QVBoxLayout *vbox = new QVBoxLayout;
            QLabel *label = new QLabel;
            label->setAlignment(Qt::AlignCenter);
            label->setPixmap(*form_disk->ui->label_icon->pixmap());
            vbox->addWidget(label);
            label = new QLabel(form_disk->name);
            label->setAlignment(Qt::AlignCenter);
            vbox->addWidget(label);
            QHBoxLayout *hbox = new QHBoxLayout;
            label = new QLabel(form_disk->device);
            hbox->addWidget(label);
            int value = 100 * (form_disk->bytesTotal - form_disk->bytesFree) / form_disk->bytesTotal;
            label = new QLabel(QString::number(value) +" %");
            label->setAlignment(Qt::AlignRight);
            hbox->addWidget(label);
            vbox->addLayout(hbox);
            QProgressBar *progressBar = new QProgressBar;
            progressBar->setRange(0,100);
            progressBar->setValue(value);
            progressBar->setFixedHeight(10);
            progressBar->setFormat("");
            QString style;
            if(value < 60){
                style = "QProgressBar { border: 1px solid gray; border-radius: 5px; background: white; }"
                        "QProgressBar::chunk { background-color: dodgerblue; }";
            }else if(value < 80){
                style = "QProgressBar { border: 1px solid gray; border-radius: 5px; background: white; }"
                        "QProgressBar::chunk { background-color: orange; }";
            }else{
                style = "QProgressBar { border: 1px solid gray; border-radius: 5px; background: white; }"
                        "QProgressBar::chunk { background-color: red; }";
            }
            progressBar->setStyleSheet(style);
            vbox->addWidget(progressBar);
            QGridLayout *gridLayout = new QGridLayout;
            label = new QLabel("文件系统");
            gridLayout->addWidget(label,0,0,Qt::AlignRight);
            label = new QLabel(form_disk->fileSystemType);
            gridLayout->addWidget(label,0,1);
            label = new QLabel("总容量");
            gridLayout->addWidget(label,1,0,Qt::AlignRight);
            label = new QLabel(BS(form_disk->bytesTotal,2));
            gridLayout->addWidget(label,1,1);
            label = new QLabel("已用容量");
            gridLayout->addWidget(label,2,0,Qt::AlignRight);
            label = new QLabel(BS(form_disk->bytesTotal - form_disk->bytesFree,2));
            gridLayout->addWidget(label,2,1);
            label = new QLabel("可用容量");
            gridLayout->addWidget(label,3,0,Qt::AlignRight);
            label = new QLabel(BS(form_disk->bytesFree,2));
            gridLayout->addWidget(label,3,1);
            vbox->addLayout(gridLayout);
            dialog->setLayout(vbox);
            dialog->exec();
        }else{
            QDialog *dialog = new QDialog;
            dialog->setWindowTitle("属性");
            dialog->setFixedSize(200,100);
            QVBoxLayout *vbox = new QVBoxLayout;
            QLabel *label = new QLabel("隐藏的分区");
            vbox->addWidget(label);
            QString partition_hide = settings.value("partition_hide", "").toString();
            if (partition_hide != "") {
                QStringList SL_partition_hide = partition_hide.split(";");
                for (int i=0; i<SL_partition_hide.length(); i++) {
                    QCheckBox *checkBox = new QCheckBox(SL_partition_hide.at(i), this);
                    checkBox->setChecked(true);
                    connect(checkBox, static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged), [=](int state){
                        if (state == Qt::Checked) {
                            settings.sync();
                            QString partition_hide1 = settings.value("partition_hide", "").toString();
                            if(partition_hide1 == ""){
                                partition_hide1 = checkBox->text();
                            }else{
                                partition_hide1 = partition_hide1 + ";" + checkBox->text();
                            }
                            settings.setValue("partition_hide", partition_hide1);
                        } else if (state == Qt::PartiallyChecked) {

                        } else if (state == Qt::Unchecked) {
                            settings.sync();
                            QString partition_hide1 = settings.value("partition_hide", "").toString();
                            QStringList SL_partition_hide1 = partition_hide1.split(";");
                            if(SL_partition_hide1.contains(checkBox->text())){
                                for(int j=0; j<SL_partition_hide1.length(); j++){
                                    if(SL_partition_hide1.at(j) == checkBox->text()){
                                        SL_partition_hide1.removeAt(j);
                                    }
                                }
                            }
                            QString s = "";
                            for(int j=0; j<SL_partition_hide1.length(); j++){
                                if(j < SL_partition_hide1.length() - 1){
                                    s += SL_partition_hide1.at(j) + ";";
                                }else{
                                    s += SL_partition_hide1.at(j);
                                }
                            }
                            settings.setValue("partition_hide", s);

                        }
                    });
                    vbox->addWidget(checkBox);
                }
            }
            QPushButton *pushButton_confirm = new QPushButton("确定");
            QPushButton *pushButton_cancel = new QPushButton("取消");
            QHBoxLayout *hbox = new QHBoxLayout;
            hbox->addStretch();
            hbox->addWidget(pushButton_confirm);
            hbox->addWidget(pushButton_cancel);
            hbox->addStretch();
            vbox->addLayout(hbox);
            dialog->setLayout(vbox);
            connect(pushButton_confirm, SIGNAL(clicked()), dialog, SLOT(accept()));
            connect(pushButton_cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
            if(dialog->exec() == QDialog::Accepted){
                genHomePage();
            }
            dialog->close();
            return;
        }
    }

}

void MainWindow::wheelEvent(QWheelEvent *e)
{
    if(QApplication::keyboardModifiers() == Qt::ControlModifier){
        qDebug() << ui->listWidget->iconSize() << ui->listWidget->gridSize();
        if(e->delta() > 0){
            zoomIn();
        }else{
            zoomOut();
        }
    }
}

void MainWindow::zoomIn()
{
    if(!ui->listWidget->isHidden())
        if(ui->listWidget->gridSize().width() < 400){
            ui->listWidget->setIconSize(QSize(ui->listWidget->iconSize().width()+30, ui->listWidget->iconSize().height()+30));
            //ui->listWidget->setGridSize(QSize(ui->listWidget->gridSize().width()+20, ui->listWidget->gridSize().height()+20));
            ui->listWidget->setGridSize(2*ui->listWidget->iconSize());
        }
}

void MainWindow::zoomOut()
{
    if(!ui->listWidget->isHidden())
        if(ui->listWidget->gridSize().width() > 80){
            ui->listWidget->setIconSize(QSize(ui->listWidget->iconSize().width()-30, ui->listWidget->iconSize().height()-30));
            //ui->listWidget->setGridSize(QSize(ui->listWidget->gridSize().width()-20, ui->listWidget->gridSize().height()-20));
            ui->listWidget->setGridSize(2*ui->listWidget->iconSize());
        }
}

void MainWindow::zoom1()
{
    if(!ui->listWidget->isHidden()){
        ui->listWidget->setIconSize(QSize(50, 50));
        ui->listWidget->setGridSize(QSize(100, 100));
    }
}

QString MainWindow::BS(qint64 b, int digits)
{
    QString s = "";
    if (b > 999999999) {
        s = QString::number(b/(1024*1024*1024.0), 'f', digits) + " GB";
    } else {
        if (b > 999999){
            s = QString::number(b/(1024*1024.0), 'f', digits) + " MB";
        } else {
            if (b > 999) {
                s = QString::number(b/1024.0, 'f', digits) + " KB";
            } else {
                s = QString::number(b)+" B";
            }
        }
    }
    return s;
}

void MainWindow::enterOpen()
{
    qDebug() << sender()->objectName();
    if (sender()->objectName().contains("ListWidget")) {
        if(ui->listWidget->currentRow() != -1){
            open(list.at(ui->listWidget->currentRow()).absoluteFilePath());
        }
    } else if (sender()->objectName().contains("TableWidget")) {
        if(ui->tableWidget->currentRow() != -1){
            open(list.at(ui->tableWidget->currentRow()).absoluteFilePath());
        }
    }
}

void MainWindow::search()
{
    qDebug() << "search";
    genList(path);
}

void MainWindow::trashFiles()
{
    if (!QDir(dirTrash).exists())
        QDir().mkpath(dirTrash);
    if (!QDir(dirTrashInfo).exists())
        QDir().mkpath(dirTrashInfo);
    modelIndexList = ui->listWidget->selectionModel()->selectedIndexes();
    foreach (QModelIndex modelIndex, modelIndexList) {
        QString filepath = modelIndex.data(LOCATION_OF_REAL_PATH).toString();
        qDebug() << "trash" << filepath;
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
        QFile file(filepath);
        if (file.moveToTrash())
            genList(path);
        else
            QMessageBox::critical(nullptr, "错误", "删除失败\n" + filepath);
#else
        QString MIME = QMimeDatabase().mimeTypeForFile(filepath).name();
        QString newName = dirTrash + "/" + QFileInfo(filepath).fileName();
        if(QFileInfo(filepath).isSymLink()){
            QFile file(newName);
            bool b = file.link(QFileInfo(filepath).symLinkTarget());
            qDebug() << newName << QFileInfo(filepath).symLinkTarget() << b;
            if (b) {
                QString pathinfo = dirTrashInfo + "/" + QFileInfo(filepath).fileName() + ".trashinfo";
                QFile filei(pathinfo);
                if (filei.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream stream(&filei);
                    QDateTime time;
                    time = QDateTime::currentDateTime();
                    stream << "[Trash Info]\nPath=" + filepath + "\nDeletionDate=" + time.toString("yyyy-MM-ddThh:mm:ss");
                    filei.close();
                }
                //if (!QFile::remove(filepath)) {
                //   QMessageBox::critical(NULL, "错误", "无法删除链接 " + filepath);
                //}
                genList(path);
            } else {
                QMessageBox::critical(this, "错误", "无法在回收站创建链接\n" + file.errorString());
            }
        } else {
            if (MIME == "inode/directory") {
                QDir *dir = new QDir;
                if (dir->rmdir(filepath)) {
                    genList(path);
                } else {
                    QMessageBox::critical(this, "错误", "无法删除文件夹\n" + filepath);
                }
            } else {
                if (QFile::copy(filepath, newName)) {
                    genList(path);
                    QString pathinfo = QDir::homePath() + "/.local/share/Trash/info/" + QFileInfo(filepath).fileName() + ".trashinfo";
                    QFile file(pathinfo);
                    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream stream(&file);
                        QDateTime time;
                        time = QDateTime::currentDateTime();
                        stream << "[Trash Info]\nPath=" + filepath + "\nDeletionDate=" + time.toString("yyyy-MM-ddThh:mm:ss");
                        file.close();
                    }
                    if (!QFile::remove(filepath)) {
                        QMessageBox::critical(nullptr, "错误", "无法删除文件 " + filepath);
                    }
                } else {
                    QMessageBox::critical(nullptr, "错误", "无法移动 " + filepath + " 到回收站");
                }
            }
        }
#endif
    }
}

void MainWindow::deleteFiles()
{
    modelIndexList = ui->listWidget->selectionModel()->selectedIndexes();
    foreach (QModelIndex modelIndex, modelIndexList) {
        QString filepath = modelIndex.data(LOCATION_OF_REAL_PATH).toString();
        qDebug() << "delete" << filepath;
        QString MIME = QMimeDatabase().mimeTypeForFile(filepath).name();
        if (MIME == "inode/directory") {
            QDir *dir = new QDir;
            if (!dir->rmdir(filepath)) {
                QMessageBox::critical(this, "错误", "无法删除文件夹 " + filepath);
            }
        } else {
            if (QFile::remove(filepath)) {
                if (QFileInfo(filepath).absolutePath() == dirTrash)
                    QFile::remove(dirTrashInfo + "/" + QFileInfo(filepath).fileName() + ".trashinfo");
            } else {
                QMessageBox::critical(nullptr, "错误", "无法删除文件 " + filepath);
            }
        }
    }
    genList(path);
}

// 递归删除文件夹 https://blog.csdn.net/u013399898/article/details/50382808
bool MainWindow::delDir(QString dirpath)
{
    ui->statusBar->showMessage("删除文件夹 " + dirpath);
    QDir dir(dirpath);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
    QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
    foreach (QFileInfo file, fileList){ //遍历文件信息
        if (file.isFile()) { // 是文件，删除
            ui->statusBar->showMessage("删除文件 " + file.fileName());
            bool b = file.dir().remove(file.fileName());
            if(file.dir() == dirTrash && b)
                QFile::remove(dirTrashInfo + "/" + file.fileName() + ".trashinfo");
        } else { // 递归删除
            delDir(file.absoluteFilePath());
        }
    }
    if(dirpath != dirTrash || dirpath != dirTrashInfo)
        return dir.rmpath(dir.absolutePath()); // 删除文件夹
    return true;
}

void MainWindow::trashDelete()
{
    if (path == dirTrash) {
        deleteFiles();
    } else {
        trashFiles();
    }
}

void MainWindow::copy()
{
    qDebug() << "copy";
    QList<QListWidgetItem*> selected_items = ui->listWidget->selectedItems();
    SL_selected_files.clear();
    for(int i=0; i<selected_items.length(); i++){
        QString fp = selected_items.at(i)->data(LOCATION_OF_REAL_PATH).toString();
        //QFileInfo fileInfo = selected_items.at(i)->data(FILE_INFO);
        qDebug() << "copy:add(" << fp << ")";
        SL_selected_files.append(fp);
    }
}

void MainWindow::paste()
{
    qDebug() << "paste";
    for(int i=0; i<SL_selected_files.length(); i++){
        source = SL_selected_files.at(i);
        QString newName = path + "/" + QFileInfo(source).fileName();
        qDebug() << "paste" << source << newName << "symLink(" << QFileInfo(source).isSymLink() << ")";
        if(QFileInfo(source).isSymLink()){
            QString symLinkTarget = QFileInfo(source).symLinkTarget();
            QFile file(symLinkTarget);
            if(!file.link(newName)){
                QMessageBox::critical(nullptr, "错误", symLinkTarget + "\n创建链接\n" + newName + "\n失败！");
                return;
            }
            if(cut){//如果是剪切
                if(!QFile::remove(source)){
                    QMessageBox::critical(nullptr, "错误", "无法删除剪切的源文件 " + source, QMessageBox::Ok);
                    return;
                }
            }
            cut = 0;
            genList(path);
        }else{
            if(!QFile::copy(source, newName)){
                QMessageBox::StandardButton SB = QMessageBox::warning(NULL, "覆盖", "是否覆盖 " + newName + " ?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
                if(SB == QMessageBox::Yes){
                    if(!QFile::remove(newName)){
                        QMessageBox::critical(nullptr, "错误", "无法覆盖新文件 " + newName);
                    }
                    if(!QFile::copy(source, newName)){
                        QMessageBox::critical(nullptr, "错误", "粘贴失败！");
                    }
                }else if(SB == QMessageBox::No){
                    newName =  path + "/副本-" + QFileInfo(newName).fileName();
                    qDebug() << "副本" << source << newName;
                    if(!QFile::copy(source, newName)){
                        QMessageBox::critical(nullptr, "错误", "粘贴失败！");
                    }
                }
            }else{
                // 修改文件时间
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
                QFile file(newName);
                qDebug() << "修改粘贴文件时间为原文件时间" << file.setFileTime(QFileInfo(source).lastModified(), QFileDevice::FileModificationTime);
#endif
                if(cut){//如果是剪切
                    if(!QFile::remove(source)){
                        QMessageBox::critical(nullptr, "错误", "无法删除剪切的源文件 " + source, QMessageBox::Ok);
                    }
                }
                cut=0;
                genList(path);
            }
        }
    }
}

void MainWindow::genList(QString spath)
{
    qDebug() << "genList" << spath;
    appendLog("genList(" + spath + ")");
    if (spath == "trash:///")
        spath = dirTrash;
    else
        lineEditLocation->setText(spath);

    for (int i=0; i<ui->listWidget_nav->count(); i++) {
        QString LWI_path = ui->listWidget_nav->item(i)->data(LOCATION_OF_REAL_PATH).toString();
        //qDebug() << "listWidget_nav.path" << LWI_path << spath.contains(LWI_path);
        if (LWI_path != "/" && LWI_path != QDir::homePath() && spath.contains(LWI_path)) {
            ui->listWidget_nav->setCurrentRow(i);
            break;
        }
    }

    QTime time;
    time.start();
    ui->listWidget->clear();
    ui->tableWidget->setRowCount(0);
    // 读取文件夹下所有文件 https://www.cnblogs.com/findumars/p/6006129.html
    QDir dir(spath);
    if (isShowHidden) {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden);
    } else {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    }
    dir.setSorting(sortFlags);
    list.clear();
    list = dir.entryInfoList();
    for (int i = 0; i < list.size(); i++) {
        QString sname="", TAG="", Title="", Artist="", Album="", Year="", Comment="";
        QFileInfo fileInfo = list.at(i);
        sname = fileInfo.fileName();
        if (sname.contains(lineEdit_search->text()) || lineEdit_search->text() == ""){
            QIcon icon;
            QString MIME = QMimeDatabase().mimeTypeForFile(fileInfo.absoluteFilePath()).name();
            //QString filetype = MIME.left(MIME.indexOf("/"));
            if (MIME == "application/x-desktop") {
                sname = readSettings(fileInfo.absoluteFilePath(), "Desktop Entry", "Name");
                QString sicon = readSettings(fileInfo.absoluteFilePath(), "Desktop Entry", "Icon");
                //qDebug() << sicon;
                if (sicon == "")
                    sicon = "applications-system-symbolic";
                if (QFileInfo(sicon).isFile()) {
                    icon = QIcon(sicon);
                } else {
                    icon = QIcon::fromTheme(sicon);
                }
            } else if (MIME == "inode/directory") {
                icon = QIcon::fromTheme("folder");
            } else if (fileInfo.suffix() == "mp3") {
                QFile file(fileInfo.absoluteFilePath());
                file.open(QIODevice::ReadOnly);
                QString ID3,Ver,Revision,Flag;
                bool ok;
                qint64 size;
                ID3 = QString(file.read(3));
                icon = QIcon::fromTheme(MIME.replace("/","-"));
                QTextCodec *TC = QTextCodec::codecForName("GBK");
                if (ID3 == "ID3") {
                    Ver = QString::number(file.read(1).toHex().toInt(&ok,16));
                    Revision = QString::number(file.read(1).toHex().toInt(&ok,16));
                    Flag = QString::number(file.read(1).toHex().toInt(&ok,16));
                    //size = file.read(4).toHex().toLongLong(&ok,16);
                    size = (file.read(1).toHex().toInt(&ok,16) & 0xEF) << 21 | (file.read(1).toHex().toInt(&ok,16) & 0xEF) << 14 | (file.read(1).toHex().toInt(&ok,16) & 0xEF) << 7 | file.read(1).toHex().toInt(&ok,16) & 0xEF;
                    while (file.pos() < size) {
                        QString FTag(file.read(4));
                        qint64 FSize = file.read(4).toHex().toLongLong(&ok,16);
                        //qint64 FSize = file.read(1).toHex().toInt(&ok,16) << 24 | file.read(1).toHex().toInt(&ok,16) << 16 | file.read(1).toHex().toInt(&ok,16) << 8 | file.read(1).toHex().toInt(&ok,16);
                        Flag = QString::number(file.read(2).toHex().toInt(&ok,16));
                        QByteArray BA = file.read(FSize);
                        if (FTag == "APIC") {
                            BA = BA.right(FSize-14);
                            QPixmap pixmap;
                            ok = pixmap.loadFromData(BA);
                            if(ok){
                                icon = QIcon(pixmap);
                            }
                            break;
                        } else if (FTag == "TYER") {
                            if(BA.contains("\xFF\xFE")){
                                Year = QString::fromUtf16(reinterpret_cast<const ushort*>(BA.mid(3,FSize-3).data()));
                            }else{
                                Year = BA.mid(1,FSize-2);
                            }
                        } else if (FTag == "COMM") {
                            QString language = BA.mid(1,3);
                            Comment = language + " "+ QString::fromUtf16(reinterpret_cast<const ushort*>(BA.mid(10,FSize-12).data()));
                        } else if (FTag == "TIT2") {
                            QByteArray UFlag = BA.left(1);
                            if(UFlag.toHex().toInt() == 0){
                                Title = TC->toUnicode(BA);
                            }else{
                                Title = QString::fromUtf16(reinterpret_cast<const ushort*>(BA.right(FSize-3).data()));
                            }
                        }else if (FTag == "TPE1") {
                            QByteArray UFlag = BA.left(1);
                            if (UFlag.toHex().toInt() == 0) {
                                Artist = TC->toUnicode(BA);
                            } else {
                                Artist = QString::fromUtf16(reinterpret_cast<const ushort*>(BA.right(FSize-3).data()));
                            }
                        }else if (FTag == "TALB") {
                            QByteArray UFlag = BA.left(1);
                            if (UFlag.toHex().toInt() == 0) {
                                Album = TC->toUnicode(BA);
                            } else {
                                Album = QString::fromUtf16(reinterpret_cast<const ushort*>(BA.right(FSize-3).data()));
                            }
                        }
                    }
                }
                qint64 pos = file.size()-128;
                file.seek(pos);
                TAG = QString(file.read(3));
                if (TAG == "TAG") {
                    Title = TC->toUnicode(file.read(30));
                    Artist = TC->toUnicode(file.read(30));
                    Album = TC->toUnicode(file.read(30));
                    Year = QString(file.read(4));
                    Comment = TC->toUnicode(file.read(28));
                }
            } else {
                icon = QIcon::fromTheme(MIME.replace("/", "-"));
                if(icon.isNull()){
                    icon = QIcon::fromTheme("unknown");
                }
            }

            if(fileInfo.isSymLink()){
                QPixmap pixmap = icon.pixmap(200, 200, QIcon::Normal, QIcon::On);
                QPainter painter(&pixmap);
                painter.drawPixmap(100, 100, QIcon::fromTheme("emblem-symbolic-link").pixmap(100, 100, QIcon::Normal, QIcon::On));
                icon = QIcon(pixmap);
            }

            QListWidgetItem *LWI;
            LWI = new QListWidgetItem(icon, sname);
            LWI->setData(LOCATION_OF_REAL_PATH, fileInfo.absoluteFilePath());
            //LWI->setData(FILE_INFO, fileInfo);
            LWI->setSizeHint(QSize(100,100));
            ui->listWidget->insertItem(i, LWI);
            ui->statusBar->showMessage("正在预览：" + QString::number(i) + "/" + QString::number(list.size()));
            ui->tableWidget ->insertRow(i);
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QIcon(icon),sname));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(fileInfo.lastModified().toString("yyyy/MM/dd HH:mm:ss")));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(BS(fileInfo.size(),2)));
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QMimeDatabase().mimeTypeForFile(fileInfo.filePath()).name()));
            ui->tableWidget->setItem(i, 4, new QTableWidgetItem(Title));
            ui->tableWidget->setItem(i, 5, new QTableWidgetItem(Artist));
            ui->tableWidget->setItem(i, 6, new QTableWidgetItem(Album));
            ui->tableWidget->setItem(i, 7, new QTableWidgetItem(Year));
            ui->tableWidget->setItem(i, 8, new QTableWidgetItem(Comment));
        }
    }
    ui->tableWidget->resizeColumnsToContents();
    iconPreview(0);
    isPreviewFinish = false;
    if (spath == dirTrash) {
        ui->statusBar->showMessage(QString::number(list.size()) + " 项目");
    } else {
        ui->statusBar->showMessage("预览 " + QString::number(list.size()) + " 个文件，耗时： " + QString::number(time.elapsed()/1000) + " 秒");
    }
}

QString MainWindow::readSettings(QString path, QString group, QString key)
{
    QSettings settings(path, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.beginGroup(group);
    QString value = settings.value(key).toString();
    return value;
}

void MainWindow::verticalScrollBarValueChanged(int v)
{
    // 图片缩略图，硬盘读取高，分页读取
    qDebug() << v << verticalScrollBar->maximum();
    if (v == verticalScrollBar->maximum()) isPreviewFinish = true;
    if (!isPreviewFinish) iconPreview(v);
}

void MainWindow::iconPreview(int v)
{
    int rowCount = ui->listWidget->height() / ui->listWidget->gridSize().height();
    int columnCount = ui->listWidget->width() / ui->listWidget->gridSize().width();
    int itemCount = rowCount * columnCount;
    qDebug() << "itemCount" << itemCount;
    int scrolledRow = v/ui->listWidget->gridSize().height();
    qDebug() << "scrolledRow" << scrolledRow;
    int startItem = scrolledRow * columnCount;
    int endItem = startItem + itemCount;
    qDebug() << "start:" << startItem << "end:" << endItem;
    for (int i = startItem; i < endItem; i++) {
        if (i < list.size()) {
            QFileInfo fileInfo = list.at(i);
            //QPixmap pixmap;
            QString MIME = QMimeDatabase().mimeTypeForFile(fileInfo.absoluteFilePath()).name();
            QString filetype = MIME.left(MIME.indexOf("/"));
            if (filetype == "image") {
                //QImageReader reader(fileInfo.absoluteFilePath());
                //reader.setAutoTransform(true);
                //QImage image = reader.read();
                //ui->listWidget->item(i)->setIcon(QIcon(QPixmap::fromImage(image.scaled(QSize(200,200), Qt::KeepAspectRatio))));
                IconPreview *iconPreview = new IconPreview();
                ui->listWidget->item(i)->setIcon(iconPreview->getIcon(fileInfo.absoluteFilePath()));
            }
        }
    }
}

void MainWindow::listWidgetItemSelectionChanged()
{
    qDebug() << "listWidgetItemSelectionChanged()" << ui->listWidget->currentItem();
}

void MainWindow::switchHidden()
{
    isShowHidden = !isShowHidden;
    genList(path);
}

void MainWindow::refresh()
{
    if(path == "computer://"){
        genHomePage();
    }else{
        genList(path);
    }
}

void MainWindow::rename()
{
    QList<QListWidgetItem*> list = ui->listWidget->selectedItems();
    foreach(QListWidgetItem *LWI, list) {
        //LWI->setFlags(LWI->flags() | Qt::ItemIsEditable);
        ui->listWidget->openPersistentEditor(LWI);
    }
}

void MainWindow::listWidgetItemChanged(QListWidgetItem *LWI)
{
    //QListWidgetItem *LWI = ui->listWidget->currentItem();
    //LWI->setFlags(LWI->flags() & (~Qt::ItemIsEditable));
    //QString oldName = LWI->text();
    QString oldName = ui->listWidget->currentItem()->text();
    QString oldPath = path + "/" + oldName;
    QString newPath = path + "/" + LWI->text();
    qDebug() << "rename" << oldPath << newPath;
    if (QFile::rename(oldPath, newPath)) {
        genList(path);
    }else{
        LWI->setText(oldName);
        QMessageBox::critical(nullptr, "错误", oldName + " 无法重命名为 " + LWI->text() + " ！", QMessageBox::Ok);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::genHomePage()
{
    ui->listWidget_nav->clear();
    ui->listWidget_userdir->clear();
    ui->listWidget_partition->clear();
    //左侧导航列表
    QListWidgetItem *LWI;
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-home"),"主目录");
    LWI->setData(LOCATION_OF_REAL_PATH, QDir::homePath());
    ui->listWidget_nav->insertItem(0, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-desktop"),"桌面");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    ui->listWidget_nav->insertItem(1, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-videos"),"视频");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    ui->listWidget_nav->insertItem(2, LWI);
    ui->listWidget_userdir->insertItem(0, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-pictures"),"图片");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    ui->listWidget_nav->insertItem(3, LWI);
    ui->listWidget_userdir->insertItem(1, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-music"),"音乐");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    ui->listWidget_nav->insertItem(4, LWI);
    ui->listWidget_userdir->insertItem(2, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-documents"),"文档");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    ui->listWidget_nav->insertItem(5, LWI);
    ui->listWidget_userdir->insertItem(3, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-downloads"),"下载");
    LWI->setData(LOCATION_OF_REAL_PATH,  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    ui->listWidget_nav->insertItem(6, LWI);
    ui->listWidget_userdir->insertItem(4, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("user-trash"),"回收站");
    LWI->setData(LOCATION_OF_REAL_PATH, QDir::homePath() + "/.local/share/Trash/files");
    ui->listWidget_nav->insertItem(7, LWI);
    LWI = new QListWidgetItem(style()->standardIcon(QStyle::SP_ComputerIcon),"计算机");
    LWI->setData(LOCATION_OF_REAL_PATH, "computer://");
    ui->listWidget_nav->insertItem(8, LWI);
    LWI = new QListWidgetItem(style()->standardIcon(QStyle::SP_DriveHDIcon),"系统盘");
    LWI->setData(LOCATION_OF_REAL_PATH, "/");
    ui->listWidget_nav->insertItem(9, LWI);
    //connect(ui->listWidget_nav, SIGNAL(clicked(QModelIndex)), this, SLOT(nav(QModelIndex)));
    connect(ui->listWidget_nav, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(nav(QListWidgetItem*)));

    //首页
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-videos"),"视频");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    ui->listWidget_userdir->insertItem(0, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-pictures"),"图片");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    ui->listWidget_userdir->insertItem(1, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-music"),"音乐");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    ui->listWidget_userdir->insertItem(2, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-documents"),"文档");
    LWI->setData(LOCATION_OF_REAL_PATH, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    ui->listWidget_userdir->insertItem(3, LWI);
    LWI = new QListWidgetItem(QIcon::fromTheme("folder-downloads"),"下载");
    LWI->setData(LOCATION_OF_REAL_PATH,  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    ui->listWidget_userdir->insertItem(4, LWI);
    connect(ui->listWidget_userdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(nav(QListWidgetItem*)));
    connect(ui->listWidget_partition, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(nav(QListWidgetItem*)));

    //系统盘
    QStorageInfo storage("/");
    QString mountPath = storage.rootPath();
    LWI = new QListWidgetItem(QIcon::fromTheme("drive-harddisk"), "系统盘");
    LWI->setData(LOCATION_OF_REAL_PATH, mountPath);
    Form_disk *form_disk = new Form_disk;
    form_disk->ui->label->setText("系统盘");
    form_disk->mountPath = mountPath;
    form_disk->device = storage.device();
    form_disk->fileSystemType = storage.fileSystemType();
    form_disk->bytesFree = storage.bytesFree();
    form_disk->bytesTotal = storage.bytesTotal();
    form_disk->init();
    LWI = new QListWidgetItem(ui->listWidget_partition);
    LWI->setSizeHint(QSize(120,130));
    LWI->setData(LOCATION_OF_REAL_PATH, mountPath);
    ui->listWidget_partition->setItemWidget(LWI, form_disk);
    ui->listWidget_partition->addItem(LWI);
    //获取挂载路径
    settings.sync();
    QString partition_hide = settings.value("partition_hide", "").toString();
    //qDebug() << partition_hide;
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            //qDebug() << "mount" << storage.name();
            QString name = storage.name();
            if(name != ""){
                bool isHidePartition = false;
                if(partition_hide != ""){
                    QStringList SL_partition_hide = partition_hide.split(";");
                    for(int i=0; i<SL_partition_hide.length(); i++){
                        if(name == SL_partition_hide.at(i)){
                            isHidePartition = true;
                        }
                    }
                }
                if(!isHidePartition){
                    QString mountPath = storage.rootPath();
                    LWI = new QListWidgetItem(QIcon::fromTheme("drive-harddisk"), name);
                    LWI->setData(LOCATION_OF_REAL_PATH, mountPath);
                    ui->listWidget_nav->insertItem(ui->listWidget_nav->count(), LWI);
                    Form_disk *form_disk = new Form_disk;
                    form_disk->name = name;
                    form_disk->mountPath = mountPath;
                    form_disk->device = storage.device();
                    form_disk->fileSystemType = storage.fileSystemType();
                    form_disk->bytesFree = storage.bytesFree();
                    form_disk->bytesTotal = storage.bytesTotal();
                    form_disk->init();
                    LWI = new QListWidgetItem(ui->listWidget_partition);
                    LWI->setSizeHint(QSize(120,130));
                    LWI->setData(LOCATION_OF_REAL_PATH, mountPath);
                    ui->listWidget_partition->setItemWidget(LWI, form_disk);
                    ui->listWidget_partition->addItem(LWI);
                }
            }
        }
    }

    ui->listWidget_nav->setCurrentRow(8);
    path = "computer://";
    lineEditLocation->setText(path);
    ui->listWidget->hide();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    QList<QListWidgetItem*> LWIs = ui->listWidget->selectedItems();
    if(LWIs.size() > 0)
        ui->listWidget->scrollToItem(LWIs.at(0), QAbstractItemView::PositionAtCenter);
}

void MainWindow::processOutput()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    QString s = QString(process->readAllStandardOutput());
    qDebug() << s;
}

void MainWindow::processError()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    QString s = QString(process->readAllStandardError());
    qDebug() << s;
}

void MainWindow::appendLog(QString s)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    s = currentDateTime.toString("yyyy/MM/dd HH:mm:ss") + " : " + s + "\n";
    QString path1 = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir dir(path1);
    if (!dir.exists())
        dir.mkpath(path1);
    path1 = path1 + "/HTYFileManager.log";
    qDebug() << path1;
    QFile file(path1);
    if (file.open(QFile::WriteOnly | QFile::Append)) {
        file.write(s.toUtf8());
        file.close();
    }
}