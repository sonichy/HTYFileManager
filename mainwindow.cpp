#include "mainwindow.h"
#include "ui_mainwindow.h"
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
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)

#else
#include <QStandardPaths>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    ui->action_icon->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    ui->action_icon->setChecked(true);
    //ui->action_list->setIcon(style()->standardIcon(QStyle::SP_TitleBarMenuButton));
    ui->action_back->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    ui->action_forward->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    LELocation = new QLineEdit(path,this);
    ui->mainToolBar->addWidget(LELocation);
    connect(LELocation,SIGNAL(returnPressed()),this,SLOT(openL()));    

    SI1=new QStandardItem(style()->standardIcon(QStyle::SP_DirHomeIcon),"主目录");
    SI2=new QStandardItem(style()->standardIcon(QStyle::SP_DesktopIcon),"桌面");
    SI3=new QStandardItem(style()->standardIcon(QStyle::SP_TrashIcon),"回收站");
    SI4=new QStandardItem(style()->standardIcon(QStyle::SP_ComputerIcon),"计算机");
    SI5=new QStandardItem(style()->standardIcon(QStyle::SP_DriveHDIcon),"系统盘");
    SI6=new QStandardItem(style()->standardIcon(QStyle::SP_DriveHDIcon),"Host");
    SIM=new QStandardItemModel(this);
    SIM->appendRow(SI1);
    SIM->appendRow(SI2);
    SIM->appendRow(SI3);
    SIM->appendRow(SI4);
    SIM->appendRow(SI5);
    SIM->appendRow(SI6);
    ui->listViewNav->setModel(SIM);
    connect(ui->listViewNav,SIGNAL(clicked(QModelIndex)),this,SLOT(nav(QModelIndex)));

    ui->tableView->setVisible(false);
    move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);
    model=new QFileSystemModel;
    // 设置model监视的目录，其下的修改会立刻signal通知view
    path="/";
    model->setRootPath(path);
    // 没有通过过滤器的文件，true为不可用，false为隐藏
    model->setNameFilterDisables(false);
    ui->listView->setModel(model);
    ui->listView->setViewMode(QListView::IconMode);
    ui->listView->setGridSize(QSize(80,80));
    ui->listView->setIconSize(QSize(40,40));
    ui->listView->setWordWrap(true);
    // 设置view显示的目录
    ui->listView->setRootIndex(model->index(path));

    connect(ui->listView,SIGNAL(clicked(QModelIndex)),this,SLOT(info(QModelIndex)));
    connect(ui->listView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(open(QModelIndex)));
    connect(ui->listView, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(viewContextMenu(QPoint)));
    connect(new QShortcut(QKeySequence(Qt::Key_Return),this), SIGNAL(activated()),this, SLOT(enterOpen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Enter),this), SIGNAL(activated()),this, SLOT(enterOpen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace),this), SIGNAL(activated()),this, SLOT(on_action_back_triggered()));

    ui->tableView->setModel(model);
    connect(ui->tableView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(open(QModelIndex)));
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(viewContextMenuTV(QPoint)));

    dialogPD=new PropertyDesktop(this);
    connect(dialogPD,SIGNAL(accepted()),dialogPD,SLOT(saveDesktop()));

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

    QStringList Largs=QApplication::arguments();
    qDebug() << Largs;
    if(Largs.length()>1){
        LELocation->setText(Largs.at(1));
        openL();
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_changelog_triggered()
{
    QMessageBox MBox(QMessageBox::NoIcon, "更新历史", "1.0\n2017-06\n增加回车键进入文件夹，增加退格键回到上层目录。\n属性窗体增加显示系统文件默认图标。\n从主窗体中分离属性窗体的代码。\n2017-05\n右键菜单增加【在终端中打开】。\n文件夹增加深度文管和Thunar打开方式。\n修复desktop已经存在，创建desktop会追加内容的BUG。\n单击文件在状态栏显示文件的MIME。\n2017-04\n图片右键菜单增加【设为壁纸】。\n文件右键菜单增加【移动到】、【复制到】。\n增加是否覆盖对话框。\ndesktop文件属性支持打开执行路径。\nQListView、QTableView实现排序。\n图标、列表按钮实现按下效果。\n实现删除文件到回收站，从回收站还原，优化回收站菜单。\n引号括起来，解决文件名含空格双击打不开的问题。\n增加列表模式右键菜单。\n增加管理员身份打开文件或文件夹。\n双击desktop文件，读取执行参数启动程序。\n增加修改desktop文件属性。\n解决QGridLayout单元格图标居中问题。\n增加读取desktop文件属性。\n增加新建文件夹，删除新建文件夹。\n程序右键增加创建快捷方式。\n图片的右键属性增加缩略图。\n2017-03\n增加左侧导航栏。\n增加右键菜单，增加复制、剪切、删除、属性功能。\n增加QTableView以列表形式显示，按钮切换图标、列表模式。\n增加后退功能。\n使用QListView以图标形式显示。");
    MBox.exec();
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "海天鹰文件管理器 1.0\n一款基于Qt的文件管理器。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：sonichy.96.lt\n参考：\n右键菜单：http://windrocblog.sinaapp.com/?p=1016\n二级菜单：http://blog.csdn.net/u011417605/article/details/51219019\nQAction组群单选：http://qiusuoge.com/12287.html\nQListView添加项目：http://blog.csdn.net/u010142953/article/details/46694419\n修改文本：http://blog.csdn.net/caoshangpa/article/details/51775147\n获取系统文件图标：http://www.cnblogs.com/RainyBear/p/5223103.html");
    aboutMB.setIconPixmap(QPixmap(":/icon.png"));
    aboutMB.exec();
}

void MainWindow::nav(QModelIndex index)
{
    //QString newpath="";
    if(index.data().toString()=="主目录"){
        path=QDir::homePath();
    }
    if(index.data().toString()=="桌面"){
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        path=QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
#else
        path=QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
        //newpath=QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).at(0);
#endif
    }
    if(index.data().toString()=="系统盘"){
        path="/";
    }
    if(index.data().toString()=="回收站"){
        path=QDir::homePath()+"/.local/share/Trash/files";
    }
    if(index.data().toString()=="Host"){
        path="/host";
    }
    qDebug() << path;
    LELocation->setText(path);
    model->setRootPath(path);
    ui->listView->setRootIndex(model->index(path));
    ui->tableView->setRootIndex(model->index(path));
    ui->tableView->resizeColumnsToContents();//适应宽度
}

void MainWindow::open(QModelIndex index)
{
    QString newpath="";
    if(index.data().toString()!="/"){
        if(path=="/"){
            newpath = "/" + index.data().toString();
        }else{
            newpath = path + "/" + index.data().toString();
        }
    }else{
        newpath = "/";
    }    
    qDebug() << "newpath:" << newpath;
    QFileInfo FI(newpath);
    if(FI.isDir() || index.data().toString()=="/"){
        LELocation->setText(newpath);
        model->setRootPath(newpath);
        ui->listView->setRootIndex(model->index(newpath));
        ui->tableView->setRootIndex(model->index(newpath));
        ui->tableView->resizeColumnsToContents();//适应宽度
        path = newpath;
    }else{
        QProcess *proc = new QProcess;
        QString MIME= QMimeDatabase().mimeTypeForFile(newpath).name();
        qDebug() << MIME;
        QString filetype=MIME.left(MIME.indexOf("/"));        
        if(filetype=="image"){
            proc->start("deepin-image-viewer \"" + newpath + "\"");
            return;
        }
        if(filetype=="video"){
            proc->start("deepin-movie \"" + newpath + "\"");
            return;
        }
        if(filetype=="audio"){
            proc->start("deepin-music \"" + newpath + "\"");
            return;
        }
        if(MIME=="text/plain" || MIME=="text/markdown"){
            proc->start("gedit \"" + newpath + "\"");
            return;
        }
        if(MIME=="text/html"){
            proc->start("google-chrome-stable \"" + newpath + "\"");
            return;
        }
        if(MIME=="application/vnd.nokia.qt.qmakeprofile"){
            proc->start("qtcreator \"" + newpath + "\"");
            return;
        }
        if(MIME=="application/x-desktop"){
            QFile file(newpath);
            file.open(QIODevice::ReadOnly);
            while(!file.atEnd()){
                QString sl=file.readLine().replace("\n","");
                if(sl.left(sl.indexOf("=")).toLower()=="exec"){
                    QString sexec=sl.mid(sl.indexOf("=")+1);
                    qDebug() << sexec;
                    proc->start(sexec);
                    break;
                }
            }
        }
    }    
}

void MainWindow::info(QModelIndex index)
{
    QString newpath=index.data().toString();
    if(index.data().toString()!="/"){
        if(path=="/"){
            newpath = "/" + index.data().toString();
        }else{
            newpath = path + "/" + index.data().toString();
        }
    }else{
        newpath = "/";
    }
    qDebug() << newpath;
    QString MIME= QMimeDatabase().mimeTypeForFile(newpath).name();
    ui->statusBar->showMessage(MIME);

}

void MainWindow::openL()
{
    QString newpath=LELocation->text();
    QFileInfo FI(newpath);
    if(FI.isDir()){
        model->setRootPath(newpath);
        ui->listView->setRootIndex(model->index(newpath));
        path = newpath;
    }
}

void MainWindow::on_action_back_triggered()
{
    if(path!="/"){
        int n=LELocation->text().lastIndexOf("/");
        qDebug() << n;
        QString newpath="";
        if(n>0){
            newpath=LELocation->text().left(n);
        }else{
            newpath="/";
        }
        LELocation->setText(newpath);
        model->setRootPath(newpath);
        ui->listView->setRootIndex(model->index(newpath));
        ui->tableView->setRootIndex(model->index(newpath));
        path=newpath;
    }
}

void MainWindow::on_action_forward_triggered()
{

}

void MainWindow::on_action_icon_triggered()
{
    ui->action_icon->setChecked(true);
    ui->action_list->setChecked(false);
    ui->listView->setVisible(true);
    ui->tableView->setVisible(false);
}

void MainWindow::on_action_list_triggered()
{
    ui->action_list->setChecked(true);
    ui->action_icon->setChecked(false);
    ui->listView->setVisible(false);
    ui->tableView->setVisible(true);    
    ui->tableView->setRootIndex(model->index(path));
    ui->tableView->resizeColumnsToContents();//适应宽度
}

void MainWindow::viewContextMenu(const QPoint &position)
{
    QAction *action_copy,*action_cut,*action_rename,*action_trash,*action_delete,*action_restore,*action_paste,*action_newdir,*action_sort,*action_property,*action_desktop,*action_gksu,*action_copyto,*action_moveto,*action_setWallpaper,*action_terminal;
    QModelIndex index=ui->listView->indexAt(position);
    //qDebug() << "setData" << model->setData(index,QPixmap("/:icon.png"),Qt::DecorationRole);
    QString filepath=index.data(QFileSystemModel::FilePathRole).toString();
    qDebug() << filepath;
    QString filename=QFileInfo(filepath).fileName();
    QString MIME= QMimeDatabase().mimeTypeForFile(filepath).name();
    qDebug() << MIME;
    QString filetype=MIME.left(MIME.indexOf("/"));

    QList<QAction *> actions;
    QAction *action_openwith=new QAction(this);
    action_openwith->setText("打开方式");
    actions.append(action_openwith);
    QMenu *SMDirectory = new QMenu(this);
    QAction *SADFM = new QAction(SMDirectory);
    SADFM->setText("深度文管");
    SMDirectory->addAction(SADFM);
    QAction *SAThunar = new QAction(SMDirectory);
    SAThunar->setText("Thunar");
    SMDirectory->addAction(SAThunar);
    if(MIME=="inode/directory"){
        action_openwith->setMenu(SMDirectory);
    }

    action_copy=new QAction(this);
    action_copy->setText("复制");
    actions.append(action_copy);

    action_cut=new QAction(this);
    action_cut->setText("剪切");
    actions.append(action_cut);

    action_paste=new QAction(this);
    action_paste->setText("粘贴");
    actions.append(action_paste);

    action_rename=new QAction(this);
    action_rename->setText("重命名");
    actions.append(action_rename);

    action_moveto=new QAction(this);
    action_moveto->setText("移动到");
    actions.append(action_moveto);

    action_copyto=new QAction(this);
    action_copyto->setText("复制到");
    actions.append(action_copyto);

    action_trash=new QAction(this);
    action_trash->setText("移至回收站");
    actions.append(action_trash);

    action_delete=new QAction(this);
    action_delete->setText("删除");
    actions.append(action_delete);

    action_restore=new QAction(this);
    action_restore->setText("还原");
    actions.append(action_restore);

    action_newdir=new QAction(this);
    action_newdir->setText("新建文件夹");
    actions.append(action_newdir);

    action_sort=new QAction(this);
    action_sort->setText("排序方式");
    actions.append(action_sort);
    action_sort->setMenu(sortMenu);

    action_setWallpaper=new QAction(this);
    action_setWallpaper->setText("设为壁纸");
    actions.append(action_setWallpaper);

    action_property=new QAction(this);
    action_property->setText("属性");
    actions.append(action_property);

    action_terminal=new QAction(this);
    action_terminal->setText("在终端中打开");
    actions.append(action_terminal);
    actions.append(action_terminal);

    action_desktop=new QAction(this);
    action_desktop->setText("创建快捷方式");
    actions.append(action_desktop);

    action_gksu=new QAction(this);
    action_gksu->setText("以管理员身份打开");
    actions.append(action_gksu);

    if(MIME!="application/x-executable" && MIME!="application/x-shellscript" && MIME!="application/x-ms-dos-executable")action_desktop->setVisible(false);
    if(filetype!="image")action_setWallpaper->setVisible(false);

    QString dirTrash=QDir::homePath()+"/.local/share/Trash/files";
    //qDebug() << path << dirTrash;
    if(path==dirTrash)
    {
        action_trash->setVisible(false);
        action_gksu->setVisible(false);
        action_rename->setVisible(false);
        action_newdir->setVisible(false);
    }else{
        action_delete->setVisible(false);
        action_restore->setVisible(false);
    }
    if(index.isValid()){
        action_newdir->setVisible(false);
        action_sort->setVisible(false);
    }else{
        qDebug()<<"viewContextMenu: index is not valid";
        action_copy->setVisible(false);
        action_cut->setVisible(false);
        action_rename->setVisible(false);
        action_trash->setVisible(false);
        action_delete->setVisible(false);
        action_gksu->setVisible(false);
        action_restore->setVisible(false);
    }

    QAction *result_action = QMenu::exec(actions,ui->listView->mapToGlobal(position));

    if(result_action == SADFM){
        QProcess *proc = new QProcess;
        QString cmd="dde-file-manager \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

    if(result_action == SAThunar){
        QProcess *proc = new QProcess;
        QString cmd="thunar \"" + filepath + "\"";
        qDebug() << cmd;
        proc->start(cmd);
        return;
    }

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
            QMessageBox::StandardButton SB = QMessageBox::warning(NULL, "覆盖", "是否覆盖 "+newName+" ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if(SB == QMessageBox::Yes){
                if(!QFile::remove(newName)){
                    QMessageBox::critical(NULL, "错误", "无法覆盖新文件 "+newName);
                }
                if(!QFile::copy(source, newName)){
                    QMessageBox::critical(NULL, "错误", "粘贴失败！");
                }
            }
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

    if(result_action == action_trash){
        qDebug() << "trash" << filepath;
        if(MIME=="inode/directory"){
            QDir *dir=new QDir;
            if(!dir->rmdir(filepath)){
                QMessageBox::critical(this,"错误","无法删除文件夹 "+filepath);
            }
        }else{
            QString newName = QDir::homePath()+"/.local/share/Trash/files/" + QFileInfo(filepath).fileName();
            if(QFile::copy(filepath, newName)){
                QString pathinfo=QDir::homePath()+"/.local/share/Trash/info/" + QFileInfo(filepath).fileName()+".trashinfo";
                QFile file(pathinfo);
                if(file.open(QIODevice::WriteOnly|QIODevice::Text))
                {
                    QTextStream stream(&file);
                    QDateTime time;
                    time = QDateTime::currentDateTime();
                    stream<<"[Trash Info]\nPath="+filepath+"\nDeletionDate="+time.toString("yyyy-MM-ddThh:mm:ss");
                    file.close();
                }
                if(!QFile::remove(filepath)){
                    QMessageBox::critical(NULL, "错误", "无法删除文件 "+filepath);
                }
            }else{
                QMessageBox::critical(NULL, "错误", "无法移动 "+filepath+" 到回收站");
            }
        }
        return;
    }

    if(result_action == action_delete){
        qDebug() << "delete" << filepath;
        if(MIME=="inode/directory"){
            QDir *dir=new QDir;
            if(!dir->rmdir(filepath)){
                QMessageBox::critical(this,"错误","无法删除文件夹 "+filepath);
            }
        }else{
            if(QFile::remove(filepath)){
                QString dirTrash=QDir::homePath()+"/.local/share/Trash/files";
                if(QFileInfo(filepath).absolutePath()==dirTrash)QFile::remove(QDir::homePath()+"/.local/share/Trash/info/"+QFileInfo(filepath).fileName()+".trashinfo");
            }else{
                QMessageBox::critical(NULL, "错误", "无法删除文件 "+filepath);
            }
        }
        return;
    }

    if(result_action == action_restore){
        qDebug() << "restore" << filepath;
        QString spath="";
        QString pathinfo=QDir::homePath()+"/.local/share/Trash/info/" + QFileInfo(filepath).fileName()+".trashinfo";
        QFile file(pathinfo);
        file.open(QIODevice::ReadOnly);
        while(!file.atEnd()){
            QString sl=file.readLine().replace("\n","");
            //qDebug() << sl;
            if(sl.left(sl.indexOf("=")).toLower()=="path"){
                spath=sl.mid(sl.indexOf("=")+1);
                break;
            }
        }
        qDebug() << spath;
        if(QFile::copy(filepath, spath)){
            QFile::remove(filepath);
            QFile::remove(QDir::homePath()+"/.local/share/Trash/info/"+QFileInfo(filepath).fileName()+".trashinfo");
        }
        return;
    }

    if(result_action == action_newdir){
        QDir *dir=new QDir;
        if(!dir->mkdir(path+"/"+"新建文件夹")){
            QMessageBox::warning(this,"创建文件夹","文件夹已经存在！");
        }
        return;
    }

    if(result_action == action_rename){
        QDialog *dialog=new QDialog(this);
        dialog->setWindowTitle("重命名");
        QVBoxLayout *vbox=new QVBoxLayout;
        QLineEdit *lineEdit=new QLineEdit;
        lineEdit->setText(filename);
        vbox->addWidget(lineEdit);
        QHBoxLayout *hbox=new QHBoxLayout;
        QPushButton *btnConfirm=new QPushButton("确定");
        QPushButton *btnCancel=new QPushButton("取消");
        hbox->addWidget(btnConfirm);
        hbox->addWidget(btnCancel);
        vbox->addLayout(hbox);
        dialog->setLayout(vbox);
        connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(btnCancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        if(dialog->exec()==QDialog::Accepted){
            QString newName=QFileInfo(filepath).absolutePath() + "/" + lineEdit->text();
            qDebug() << "rename" << filepath << newName;
            if(!QFile::rename(filepath, newName)){
                QMessageBox::critical(NULL, "错误", "无法重命名文件，该文件已存在！", QMessageBox::Ok);
            }
        }
        dialog->close();
        return;
    }

    if(result_action == action_property){
        if(MIME=="application/x-desktop"){
            //pathDesktop=filepath;
            QString sname="",sexec="",spath="",scomment="";
            QFile file(filepath);
            file.open(QIODevice::ReadOnly);
            while(!file.atEnd()){
                QString sl=file.readLine().replace("\n","");
                //qDebug() << sl;
                if(sl.left(sl.indexOf("=")).toLower()=="name"){
                    sname=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower()=="exec"){
                    sexec=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower()=="icon"){
                    pathIcon=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower()=="path"){
                    spath=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower()=="comment"){
                    scomment=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
            }
            dialogPD->ui->lineEditPathDesktop->setText(filepath);
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
            QMessageBox MBox(QMessageBox::NoIcon, "属性", "文件名：\t"+QFileInfo(filepath).fileName()+"\n大小：\t"+BS(QFileInfo(filepath).size())+"\n类型：\t"+QMimeDatabase().mimeTypeForFile(filepath).name()+"\n访问时间：\t"+QFileInfo(filepath).lastRead().toString("yyyy-MM-dd hh:mm:ss")+"\n修改时间：\t"+QFileInfo(filepath).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            if(filetype=="image"){
                QSize iconSize(200,200);
                MBox.setIconPixmap(QPixmap(filepath).scaled(iconSize, Qt::KeepAspectRatio));
            }else{
                QFileInfo fileinfo(filepath);
                QFileIconProvider iconProvider;
                QIcon icon = iconProvider.icon(fileinfo);
                MBox.setIconPixmap(icon.pixmap(QSize(128,128)));
            }
            MBox.exec();
        }
        return;
    }

    if(result_action == action_desktop){
        qDebug() << "Create desktop file";
        QString str="[Desktop Entry]\nName="+QFileInfo(filepath).fileName()+"\nComment=\nExec="+filepath+"\nIcon="+QFileInfo(filepath).absolutePath() + "/icon.png\nPath="+QFileInfo(filepath).absolutePath()+"\nTerminal=false\nType=Application\nMimeType=\nCategories=";
        qDebug() << str;
        QFile file(QFileInfo(filepath).absolutePath()+"/"+QFileInfo(filepath).fileName() + ".desktop");
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this,"错误","不能创建 "+filepath+".desktop",QMessageBox::Yes);
        }
        QTextStream in(&file);
        in << str;
        file.close();
        return;
    }

    if(result_action == action_gksu){
        qDebug() << "gksu";
        QProcess *proc = new QProcess;
        if(filetype=="text"){
            proc->start("gksu gedit " + filepath);
        }
        if(MIME=="inode/directory"){
            proc->start("gksu " + QDir::currentPath() + "/HTYFileManager " + filepath);
        }
    }

    if(result_action == action_terminal){
        QProcess *proc = new QProcess;
        QString cmd="";
        if(index.isValid()){
            cmd = "deepin-terminal -w " + filepath;
        }else{
            cmd = "deepin-terminal -w " + path;
        }
        qDebug() << cmd;
        proc->start(cmd);
    }

    if(result_action == action_sortName){
        qDebug() << "sort by name";
        model->sort(0,Qt::AscendingOrder);
    }

    if(result_action == action_sortSize){
        qDebug() << "sort by size";
        model->sort(1,Qt::DescendingOrder);
    }

    if(result_action == action_sortType){
        qDebug() << "sort by type";
        model->sort(0,Qt::AscendingOrder);
    }

    if(result_action == action_sortTime){
        qDebug() << "sort by time";
        model->sort(3,Qt::DescendingOrder);
    }

    if(result_action == action_copyto){
        if(dir==""){
            dir = QFileDialog::getExistingDirectory(this, "选择路径", "/home", QFileDialog::ShowDirsOnly |QFileDialog::DontResolveSymlinks);
        }else{
            dir = QFileDialog::getExistingDirectory(this, "选择路径", dir, QFileDialog::ShowDirsOnly |QFileDialog::DontResolveSymlinks);
        }
        if(dir!=""){
            QString newName = dir + "/" + QFileInfo(filepath).fileName();
            qDebug() << "copyto:" << filepath << newName;
            if(!QFile::copy(filepath, newName)){
                QMessageBox::StandardButton SB = QMessageBox::warning(NULL, "覆盖", "是否覆盖 "+newName+" ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if(SB == QMessageBox::Yes){
                    if(!QFile::remove(newName)){
                        QMessageBox::critical(NULL, "错误", "无法覆盖新文件 "+newName);
                    }else{
                        qDebug() << "remove" << newName;
                    }
                    if(!QFile::copy(filepath, newName)){
                        QMessageBox::critical(NULL, "错误", "粘贴失败！");
                    }else{
                        qDebug() << "copy" << filepath << newName;
                    }
                }
            }
        }
        return;
    }

    if(result_action == action_moveto){
        if(dir==""){
            dir = QFileDialog::getExistingDirectory(this, "选择路径", "/home", QFileDialog::ShowDirsOnly |QFileDialog::DontResolveSymlinks);
        }else{
            dir = QFileDialog::getExistingDirectory(this, "选择路径", dir, QFileDialog::ShowDirsOnly |QFileDialog::DontResolveSymlinks);
        }
        if(dir!=""){
            QString newName = dir + "/" + QFileInfo(filepath).fileName();
            qDebug() << "copyto:" << filepath << newName;
            if(!QFile::copy(filepath, newName)){
                QMessageBox::StandardButton SB = QMessageBox::warning(NULL, "覆盖", "是否覆盖 "+newName+" ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if(SB == QMessageBox::Yes){
                    if(!QFile::remove(newName)){
                        QMessageBox::critical(NULL, "错误", "无法覆盖新文件 " + newName);
                    }else{
                        qDebug() << "remove" << newName;
                    }
                    if(!QFile::copy(filepath, newName)){
                        QMessageBox::critical(NULL, "错误", "粘贴失败！");
                    }else{
                        qDebug() << "copy" << filepath << newName;
                        if(!QFile::remove(filepath)){
                            QMessageBox::critical(NULL, "错误", "无法删除源文件 " + filepath);
                        }else{
                            qDebug() << "remove" << filepath;
                        }
                    }
                }
            }else{
                qDebug() << "copy" << filepath << newName;
                if(!QFile::remove(filepath)){
                    QMessageBox::critical(NULL, "错误", "无法删除源文件 " + filepath);
                }else{
                    qDebug() << "remove" << filepath;
                }
            }
        }
        return;
    }

    if(result_action == action_setWallpaper){
        QString cmd="gsettings set org.gnome.desktop.background picture-uri \"file://" + filepath + "\"";
        qDebug() << "setWallpaper:" << cmd;
        QProcess *proc = new QProcess;
        proc->start(cmd);
        return;
    }

    foreach(QAction* action, actions){
        action->deleteLater();
    }
}

void MainWindow::viewContextMenuTV(const QPoint &position)
{
    QAction *action_copy,*action_cut,*action_rename,*action_trash,*action_delete,*action_paste,*action_newdir,*action_property,*action_desktop,*action_gksu;
    QModelIndex index=ui->tableView->indexAt(position);
    QString filepath=index.data(QFileSystemModel::FilePathRole).toString();
    QString filename=QFileInfo(filepath).fileName();
    QString MIME= QMimeDatabase().mimeTypeForFile(filepath).name();
    qDebug() << MIME;
    QString filetype=MIME.left(MIME.indexOf("/"));

    QList<QAction *> actions;
    action_copy=new QAction(this);
    action_copy->setText("复制");
    actions.append(action_copy);

    action_cut=new QAction(this);
    action_cut->setText("剪切");
    actions.append(action_cut);

    action_rename=new QAction(this);
    action_rename->setText("重命名");
    actions.append(action_rename);

    action_delete=new QAction(this);
    action_delete->setText("删除");
    actions.append(action_delete);

    action_paste=new QAction(this);
    action_paste->setText("粘贴");
    actions.append(action_paste);

    action_newdir=new QAction(this);
    action_newdir->setText("新建文件夹");
    actions.append(action_newdir);

    action_property=new QAction(this);
    action_property->setText("属性");
    actions.append(action_property);

    action_desktop=new QAction(this);
    action_desktop->setText("创建快捷方式");
    actions.append(action_desktop);

    action_gksu=new QAction(this);
    action_gksu->setText("以管理员身份打开");
    actions.append(action_gksu);

    if(MIME!="application/x-executable")action_desktop->setVisible(false);

    if(!index.isValid()){
        qDebug()<<"viewContextMenu: index is not valid";
        action_copy->setVisible(false);
        action_cut->setVisible(false);
        action_rename->setVisible(false);
        action_delete->setVisible(false);
        action_gksu->setVisible(false);
    }

    QAction *result_action = QMenu::exec(actions,ui->tableView->mapToGlobal(position));

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
        if(MIME=="inode/directory"){
            QDir *dir=new QDir;
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
        QDir *dir=new QDir;
        if(!dir->mkdir(path+"/"+"新建文件夹")){
            QMessageBox::warning(this,"创建文件夹","文件夹已经存在！");
        }
        return;
    }

    if(result_action == action_rename){
        QDialog *dialog=new QDialog(this);
        dialog->setWindowTitle("重命名");
        QVBoxLayout *vbox=new QVBoxLayout;
        QLineEdit *lineEdit=new QLineEdit;
        lineEdit->setText(filename);
        vbox->addWidget(lineEdit);
        QHBoxLayout *hbox=new QHBoxLayout;
        QPushButton *btnConfirm=new QPushButton("确定");
        QPushButton *btnCancel=new QPushButton("取消");
        hbox->addWidget(btnConfirm);
        hbox->addWidget(btnCancel);
        vbox->addLayout(hbox);
        dialog->setLayout(vbox);
        connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(btnCancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        if(dialog->exec()==QDialog::Accepted){
            QString newName=QFileInfo(filepath).absolutePath() + "/" + lineEdit->text();
            qDebug() << "rename" << filepath << newName;
            if(!QFile::rename(filepath, newName)){
                QMessageBox::critical(NULL, "错误", "无法重命名文件，该文件已存在！", QMessageBox::Ok);
            }
        }
        dialog->close();
        return;
    }

    if(result_action == action_property){
        if(MIME=="application/x-desktop"){
            pathDesktop=filepath;
            QString sname="",sexec="",spath="",scomment="";
            QFile file(filepath);
            file.open(QIODevice::ReadOnly);
            while(!file.atEnd()){
                QString sl=file.readLine().replace("\n","");
                qDebug() << sl;
                if(sl.left(sl.indexOf("=")).toLower()=="name"){
                    sname=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower()=="exec"){
                    sexec=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower()=="icon"){
                    pathIcon=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower()=="path"){
                    spath=sl.mid(sl.indexOf("=")+1);
                    continue;
                }
                if(sl.left(sl.indexOf("=")).toLower()=="comment"){
                    scomment=sl.mid(sl.indexOf("=")+1);
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
            QMessageBox MBox(QMessageBox::NoIcon, "属性", "文件名：\t"+QFileInfo(filepath).fileName()+"\n大小：\t"+BS(QFileInfo(filepath).size())+"\n类型：\t"+QMimeDatabase().mimeTypeForFile(filepath).name()+"\n访问时间：\t"+QFileInfo(filepath).lastRead().toString("yyyy-MM-dd hh:mm:ss")+"\n修改时间：\t"+QFileInfo(filepath).lastModified().toString("yyyy-MM-dd hh:mm:ss"));

            if(filetype=="image"){
                QSize iconSize(200,200);
                MBox.setIconPixmap(QPixmap(filepath).scaled(iconSize, Qt::KeepAspectRatio));
            }
            MBox.exec();
        }
        return;
    }

    if(result_action == action_desktop){
        qDebug() << "Create desktop file";
        QString str="[Desktop Entry]\nName="+QFileInfo(filepath).fileName()+"\nComment=\nExec="+filepath+"\nIcon="+QFileInfo(filepath).absolutePath() + "/icon.png\nPath="+QFileInfo(filepath).absolutePath()+"\nTerminal=false\nType=Application\nMimeType=\nCategories=";
        QFile file(QFileInfo(filepath).absolutePath()+"/"+QFileInfo(filepath).fileName() + ".desktop");
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        {
            QMessageBox::warning(this,"错误","不能创建 "+filepath+".desktop",QMessageBox::Yes);
        }
        QTextStream in(&file);
        in << str;
        file.close();
        return;
    }

    if(result_action == action_gksu){
        QProcess *proc = new QProcess;
        if(filetype=="text"){
            proc->start("gksu gedit " + filepath);
        }
        if(MIME=="inode/directory"){
            proc->start("gksu " + QDir::currentPath() + "/HTYFileManager " + filepath);
        }
    }

    foreach(QAction* action, actions)
    {
        action->deleteLater();
    }
}

void MainWindow::wheelEvent(QWheelEvent *e)
{
    if(QApplication::keyboardModifiers() == Qt::ControlModifier){
        qDebug() << ui->listView->gridSize() << ui->listView->iconSize();
        if(e->delta() > 0){
            if(ui->listView->gridSize().width()<300){
                ui->listView->setGridSize(QSize(ui->listView->gridSize().width()+20,ui->listView->gridSize().height()+20));
                ui->listView->setIconSize(QSize(ui->listView->iconSize().width()+20,ui->listView->iconSize().height()+20));
            }
        }else{
            if(ui->listView->gridSize().width()>80){
                ui->listView->setGridSize(QSize(ui->listView->gridSize().width()-20,ui->listView->gridSize().height()-20));
                ui->listView->setIconSize(QSize(ui->listView->iconSize().width()-20,ui->listView->iconSize().height()-20));
            }
        }
    }
}

QString MainWindow::BS(qint64 b)
{
    QString s="";
    if(b>999999999){
        s=QString::number(b/(1024*1024*1024.0),'f',2)+" GB";
    }else{
        if(b>999999){
            s=QString::number(b/(1024*1024.0),'f',2)+" MB";
        }else{
            if(b>999){
                s=QString::number(b/1024.0,'f',2)+" KB";
            }else{
                s=QString::number(b)+" B";
            }
        }
    }
    return s;
}

void MainWindow::enterOpen()
{
    open(ui->listView->selectionModel()->selectedIndexes().at(0));
}
