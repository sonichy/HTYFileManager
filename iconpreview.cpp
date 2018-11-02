#include "iconpreview.h"
#include <QImageReader>

IconPreview::IconPreview()
{

}

QIcon IconPreview::getIcon(QString path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    return QIcon(QPixmap::fromImage(image.scaled(QSize(200,200), Qt::KeepAspectRatio)));
}