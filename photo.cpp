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
    qDebug("PhotoView::PhotoView");
    m_opacity = 1.0;
}

void PhotoView::initTimeLine()
{
    qDebug("PhotoView::initTimeLine");

    //timeline used to play effects and change photos
    m_timeLine = new QTimeLine(SLIDESHOW_DURATION, this);
    m_timeLine->setFrameRange(0, SLIDESHOW_DURATION); // set frame range

    /**
     * Constant                      Value   Description
     * QTimeLine::EaseInCurve        0       The value starts growing slowly, then increases in speed.
     * QTimeLine::EaseOutCurve       1       The value starts growing steadily, then ends slowly.
     * QTimeLine::EaseInOutCurve     2       The value starts growing slowly, the runs steadily, then grows slowly again.
     * QTimeLine::LinearCurve        3       The value grows linearly (e.g., if the duration is 1000 ms, the value at time 500 ms is 0.5).
     * QTimeLine::SineCurve          4       The value grows sinusoidally.
     **/
    m_timeLine->setCurveShape(QTimeLine::LinearCurve);

    connect(m_timeLine, SIGNAL(frameChanged(int)), this, SLOT(setValue(int)));
    connect(m_timeLine, SIGNAL(finished()), this, SLOT(nextPhoto()));

}


PhotoView::~PhotoView()
{
    qDebug("PhotoView destructed");
}

/**
*
* This function is called to start the slideshow.
*
* We will start the slideshow after 1 image is received.
*
* requestPhoto() will start the network request for the images.
*
*
*/
void PhotoView::start()
{
    initTimeLine();

    connect(this, SIGNAL(photoAdded()), this, SLOT(controlTimeLine()));

    qDebug("PhotoView::start()");
    if(!m_urlList.isEmpty()) {
	for(int i = 0; i < m_urlList.size(); i++) {
	  // start downloading the first photo
	  requestPhoto(m_urlList[i]);
	}
    }

}

/**
*
* This will run during _any_ update.
*
* So not only when called from within this class, but also when windows are moving around etc.
* Many things can trigger an update event, outside of the context of this slideshow...
*
* Actually it needs to be repainted when the paintEvent occurs..
* Which means inside this event nothing else should be going on than repainting what should be the current state.
*
*/

// http://apidocs.meego.com/1.2-preview/qt4/demos-embedded-fluidlauncher-slideshow-cpp.html
void PhotoView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    // todo make this available to the effect
    painter.setOpacity(m_opacity);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(event->rect(), Qt::black);

    if((m_index + 1) > m_imageList.size()) {
	qDebug("refusing to draw an out of range image: %i %i", m_index, m_imageList.size());
	return;
    }

    if (!m_imageList.isEmpty()) {

        //qDebug("PhotoView::PaintEvent(): Painting image %d", m_index);
        QPixmap slide = QPixmap::fromImage(m_imageList.at(m_index), Qt::AutoColor);

        QSize slideSize = slide.size();

        QSize scaledSize = QSize(qMin(slideSize.width(), size().width()), qMin(slideSize.height(), size().height()));
        //qDebug("scaledSize %dx%d", slideSize.width(), slideSize.height());

        if (slideSize != scaledSize)
            slide = slide.scaled(scaledSize, Qt::KeepAspectRatio);

        QRect pixmapRect(qMax( (size().width() - slide.width())/2, 0),
                         qMax( (size().height() - slide.height())/2, 0),
                         slide.width(),
                         slide.height());

        //qDebug("dimensions: %dx%d", slide.width(), slide.height());
        painter.drawPixmap(pixmapRect, slide);

    } else {
        qDebug("PhotoView::PaintEvent(): Paint event but imagelist is empty...");
	painter.fillRect(event->rect(), Qt::black);
    }
}

/**
*
* First we add urls to the photoView.
*
*/

void PhotoView::addPhotoUrl( const QUrl &url )
{
	m_urlList.append(url);

	qDebug() << "PhotoView::addPhotoUrl()";

}

/**
*
* Downloads a new photo 
*
*/

void PhotoView::requestPhoto ( const QUrl &url )
{
    qDebug("PhotoView::requestPhoto()");
    //setting photo into request queue
    QNetworkAccessManager *manager = new QNetworkAccessManager ( this );
    connect ( manager, SIGNAL ( finished ( QNetworkReply* ) ), this, SLOT ( addPhoto ( QNetworkReply* ) ) );

    manager->get(QNetworkRequest(url));
}

/**
*
* Photo data is received, add it to the m_imageList as a QImage
*
*/
void PhotoView::addPhoto ( QNetworkReply * reply )
{
    QImage image;
    //getting image from received binary data
    if ( image.loadFromData ( reply->readAll() ) )
    {
        m_imageList.append ( image );
	qDebug("PhotoView::addPhoto: Added image %d,(size: %dx%d", m_imageList.size(), image.width(), image.height());
    } else {
        qDebug("PhotoView::addPhoto() failed to load image data");
    }

    emit photoAdded();

}

/**
*
* This function is run during the frameChanged event of the timeline.
*
* The range defined currently is 1000.
*
* After 50 frames the photo will change.
*
* Right now the only effect is changing the opacity value.
*
* This eventually will be delegated to one of the effect classes.
*
*/
void PhotoView::setValue(int i)
{
    //change photo in the middle of fading animation
    if(i > 500 && m_changePhoto){
	qDebug("PhotoView::setValue(): change Photo");
        m_index++;
	//rolling slideshow to beginning if it reaches the end
	if ( (m_index + 1) == m_imageList.size() ) {
	  qDebug() << "rolling slideshow to beginning";
	  m_index = 0;
	}
        m_changePhoto = false;
    }

    int pause = 200;

    //fadeout <--> fadein
    if(i < (500 - pause)) {
        m_opacity = (500.0-(qreal)i)/500.0;
    } else if(i > (500 + pause)) {
        m_opacity = ((qreal)i-500.0)/500.0;
    }

    //qDebug("PhotoView::setValue(): opacity changed to %f", m_opacity);

    // update the photoView, which will dispatch frameChanged event, which will call setValue etc..
    update();
}

/**
*
* nextPhoto is called everytime the timer times out.
*
*/
void PhotoView::nextPhoto()
{
    if ( m_imageList.isEmpty() ) {
        qDebug() << "m_imageList is empty";
        return;
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

    // (re-)start the timeline for effects
    qDebug("Restarting timeline");
    m_timeLine->start();
}

/**
*
* Runs when a photo is added to the list.
* 
* - (re)-starts the timeline if it's not running
*
*/
void PhotoView::controlTimeLine()
{

    qDebug("PhotoView::controlTimeLine()");

    if(m_timeLine->state() == QTimeLine::NotRunning) {
	    m_timeLine->start();
	    qDebug("PhotoView::controlTimeLine(): start timeline");
    }


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
