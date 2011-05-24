#ifndef PHOTO_H
#define PHOTO_H

#include <QWidget>
#include <QNetworkReply>
#include <QTimer>
#include <QTimeLine>

#define MAX_PHOTOS 10
#define CACHE_PHOTOS 5

class PhotoView : public QWidget
{
    Q_OBJECT

public:
    PhotoView(QWidget * parent = 0);

    ~PhotoView();
    void requestPhoto ( const QUrl &url );
    void addPhotoUrl ( const QUrl &url );
    void initTimeLine();
    void start();

signals:
    void giveMeMore(int num);
    void inputReceived();
    void photoAdded();

private slots:
    void addPhoto ( QNetworkReply * reply );
    void setValue(int i);
    void nextPhoto();
    void controlTimeLine();

protected:
    void paintEvent ( QPaintEvent * event );
    void keyPressEvent(QKeyEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void showEvent(QShowEvent * event );

private:
    QList<QImage> m_imageList;
    QList<QUrl> m_urlList;
    int m_index;
    uint m_last_index;
    QTimer * m_timer;
    QTimeLine * m_timeLine;
    qreal m_opacity;
    bool m_changePhoto;
};

#endif
