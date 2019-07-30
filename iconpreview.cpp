#include "iconpreview.h"
#include <QImageReader>
#include <QFileInfo>
#include <QPainter>

IconPreview::IconPreview()
{

}

QIcon IconPreview::getIcon(QString path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    QPixmap pixmap = QPixmap::fromImage(image.scaled(QSize(200,200), Qt::KeepAspectRatio));
    QFileInfo FI(path);
    if(FI.isSymLink()){
        QPainter painter(&pixmap);
        painter.drawPixmap(150, 150, QIcon::fromTheme("emblem-symbolic-link").pixmap(50, 50, QIcon::Normal, QIcon::On));
    }
    return QIcon(pixmap);
}