/******************************************************************************
 *   Copyright (C) 2009 by Evgeni Gordejev                                    *
 *   evgeni.gordejev@gmail.com                                                *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU Library Lesser General Public License as   *
 *   published by the Free Software Foundation; either version 2 of the       *
 *   License, or (at your option) any later version.                          *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU Lesser General Public License for more details.                      *
 *                                                                            *
 *   You should have received a copy of the GNU Library Lesser General Public *
 *   License along with this program; if not, write to the                    *
 *   Free Software Foundation, Inc.,                                          *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                *
 ******************************************************************************/

#include "photo.h"
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include <QNetworkAccessManager>
#include <QtCore>

#define WIDGET_SIZE_HALF 120
#define WIDGET_SIZE WIDGET_SIZE_HALF*2
#define SLIDESHOW_DURATION 7000
#define FADE_DURATION 1000

PhotoView::PhotoView ( QWidget * parent )
    : QWidget ( parent ),
    m_index ( 0 )
{
    //timer used to change photos
    m_timer = new QTimer ( this );
    connect ( m_timer, SIGNAL ( timeout() ), this, SLOT ( nextPhoto() ) );
    m_timer->start ( SLIDESHOW_DURATION );

    //timeline used for fading effect
    m_timeLine = new QTimeLine(FADE_DURATION, this);
    m_timeLine->setFrameRange(0, 100);
    m_timeLine->setCurveShape(QTimeLine::LinearCurve);
    connect(m_timeLine, SIGNAL(frameChanged(int)), this, SLOT(setValue(int)));

    m_opacity = 1.0;
    //this->setFixedSize ( WIDGET_SIZE,WIDGET_SIZE );
}


PhotoView::~PhotoView()
{
}

// http://apidocs.meego.com/1.2-preview/qt4/demos-embedded-fluidlauncher-slideshow-cpp.html
void PhotoView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setOpacity(m_opacity);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (!m_imageList.isEmpty() > 0) {

	qDebug("PhotoView::PaintEvent(): Painting image %d", m_index);
        QPixmap slide = QPixmap::fromImage(m_imageList.at(m_index), Qt::AutoColor);
        QSize slideSize = slide.size();

        QSize scaledSize = QSize(qMin(slideSize.width(), size().width()), qMin(slideSize.height(), size().height()));
	qDebug("scaledSize %dx%d", slideSize.width(), slideSize.height());

        if (slideSize != scaledSize)
            slide = slide.scaled(scaledSize, Qt::KeepAspectRatio);

        QRect pixmapRect(qMax( (size().width() - slide.width())/2, 0),
                         qMax( (size().height() - slide.height())/2, 0),
                         slide.width(),
                         slide.height());

	qDebug("dimensions: %dx%d", slide.width(), slide.height());

        if (pixmapRect.top() > 0) {
            // Fill in top & bottom rectangles:
            painter.fillRect(0, 0, size().width(), pixmapRect.top(), Qt::black);
            painter.fillRect(0, pixmapRect.bottom(), size().width(), size().height(), Qt::black);
        }

        if (pixmapRect.left() > 0) {
            // Fill in left & right rectangles:
            painter.fillRect(0, 0, pixmapRect.left(), size().height(), Qt::black);
            painter.fillRect(pixmapRect.right(), 0, size().width(), size().height(), Qt::black);
        }

        painter.drawPixmap(pixmapRect, slide);

    } else {
        painter.fillRect(event->rect(), Qt::black);
    }
}

void PhotoView::addPhoto ( const QString &url )
{
    //setting photo into request queue
    QNetworkAccessManager *manager = new QNetworkAccessManager ( this );
    connect ( manager, SIGNAL ( finished ( QNetworkReply* ) ),
              this, SLOT ( replyFinished ( QNetworkReply* ) ) );

    manager->get ( QNetworkRequest ( QUrl ( url ) ) );
}

void PhotoView::replyFinished ( QNetworkReply * reply )
{
    QImage image;
    //getting image from received binary data
    if ( image.loadFromData ( reply->readAll() ) )
    {
        m_imageList.append ( image );
	qDebug("PhotoView::replyFinished: Added image %d, size: %dx%d", m_imageList.size(), image.width(), image.height());
    }
    update();
}

void PhotoView::setValue(int i)
{
    //change photo in the middle of fading animation
    if(i > 50 && m_changePhoto){
	qDebug("PhotoView::setValue(): change Photo");
        m_index++;
        m_changePhoto = !m_changePhoto;
    }

    //fadeout <--> fadein
    if(i < 50) {
        m_opacity = (50.0-(qreal)i)/50.0;
    } else {
        m_opacity = ((qreal)i-50.0)/50.0;
    }

    qDebug("PhotoView::setValue(): opacity changed to %f", m_opacity);

    update();
}

void PhotoView::nextPhoto()
{
    if ( m_imageList.isEmpty() ) {
        qDebug() << "m_imageList is empty";
        return;
    }

    //rolling slideshow to beginning if it reaches the end
    if ( m_index == m_imageList.size() ) {
        qDebug() << "rolling slideshow to beginning";
        m_index = 0;
    }

    //slideshow is in the middle
    if( m_index == (MAX_PHOTOS-CACHE_PHOTOS)){
        //remove already showed photos
        qDebug() << "removing already showed photos";
        while(m_imageList.size() > (MAX_PHOTOS-CACHE_PHOTOS)){
            m_imageList.removeFirst();
        }
        //get 5 new photos
        qDebug() << "getting new photos";
        emit giveMeMore(CACHE_PHOTOS);
        m_index = 0;
    }

    m_changePhoto = true;
    m_timeLine->start();
}

void PhotoView::keyPressEvent(QKeyEvent* event)
{
    Q_UNUSED(event);
    emit inputReceived();
}

void PhotoView::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit inputReceived();
}

void PhotoView::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit inputReceived();
}

void PhotoView::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit inputReceived();
}

void PhotoView::showEvent(QShowEvent * event )
{
    Q_UNUSED(event);
#ifndef QT_NO_CURSOR
    setCursor(Qt::BlankCursor);
#endif
}
