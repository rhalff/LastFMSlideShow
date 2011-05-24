#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QStatusBar>
#include <QProgressBar>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>
#include <QKeyEvent>
#include "photo.h"
#include "defs.h"

MainWindow::MainWindow(QString &artist) :
   m_isFullscreen(false)
{

    m_artist = (artist == "") ? "Jimi Hendrix" : artist;

    prepareHomeDir();

    /* Progress bar to signal progress of RSS feed download and web page load. */
    m_progress = new QProgressBar;
    statusBar()->addPermanentWidget(m_progress);

    m_photoView = new PhotoView();
    // TODO: reimplement this getmore function 
    //    connect ( m_photoView,SIGNAL ( giveMeMore(int) ),this,SLOT ( getPhotos(int) ) );
    this->setCentralWidget ( m_photoView );

    // set default size (not really necessary)
    resize(800,600);

    // getPhotos(MAX_PHOTOS);

    /* Set up the network manager. */
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    // fetch the rss feed from last.fm
    qDebug("Invoking fetchPhotos()");
    fetchPhotos();

}

MainWindow::~MainWindow()
{
}

/**
*
* Create the home directory, if it doesn't exist yet
*
*/
void MainWindow::prepareHomeDir()
{

#ifdef Q_WS_X11
  QDir(QDir::homePath()).mkdir(TEMP_PATH);
  m_tempStorageDir = QDir::homePath() + "/" + TEMP_PATH;
#else
  QDir::home().root().mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
  m_tempStorageDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif

}

/**
*
* Show download progress for the feed, not sure If I want to keep this.
*
*/
void MainWindow::downloadProgress(qint64 bytes, qint64 bytesTotal)
{
    if (bytesTotal == -1) {
        /* No total bytes available, just set the progress bar to show a busy indicator. */
        m_progress->setMinimum(0);
        m_progress->setMaximum(0);
    } else {
        m_progress->setMaximum(100);
        int percent = bytes * 100 / bytesTotal;
        m_progress->setValue(percent);
    }
}

/**
*
* Fetch the photo's from last.fm using their xml feed
*
*/
void MainWindow::fetchPhotos() {

    // check DB for photos

    // otherwise get it from lastFM
    QString rss_url = "http://ws.audioscrobbler.com/2.0/?method=artist.getImages&api_key=" LASTFM_API_KEY "&artist=" + m_artist;

    m_reply = manager->get(QNetworkRequest(QUrl(rss_url)));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));

}

/**
*
* Runs when the feed has finished downloading.
*
* Checks whether we have a valid XML document and passes it on to readLastFM
*
*/
void MainWindow::replyFinished(QNetworkReply * netReply)
{
    QString str (netReply->readAll());

    /* If we are redirected, try again. TODO: Limit redirection count. */
/*
    QVariant vt = netReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
*/

    qDebug() << "network reply finished";

    delete m_reply;
/*
    if (!vt.isNull()) {
        qDebug() << "Redirected to:" << vt.toUrl().toString();
        m_reply = manager->get(QNetworkRequest(vt.toUrl()));
        connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    } else {
*/
    /* We have something. */
    QDomDocument doc;
    QString error;
    if (!doc.setContent(str, false, &error)) {
	    qDebug() << error; 
    } else {

	    // TODO: check the db first

	    // get images from last.fm
	    readLastFM(doc);
    }
 //   }
}

/**
*
* Will parse the xml from last.fm
*
* Only extralarge and original images are considered
* 
* Extralarge is actually allready too small, but it will do.
*
* photo's are added to the photoView instance (m_photoView);
*
*/
void MainWindow::readLastFM(const QDomDocument &doc) const { 

	QDomElement docElem = doc.documentElement();
	QDomNodeList nodeList = docElem.elementsByTagName("sizes");

	QString artist_image_url = "";

	qDebug() << "readRSS";

	for (uint i = 0; i < nodeList.length(); i++)
	{
		QDomNode node = nodeList.item(i);
		QDomElement e = node.toElement();

		QDomNodeList sizeList = e.elementsByTagName("size");

		if(!sizeList.length()) {
			qDebug("No photos found");
		} else {

			QDomNode sizeNode;
			QDomElement sizeEl;
			for (uint j = 0; j < sizeList.length(); j++) {

				sizeNode  = sizeList.item(j);
				sizeEl = sizeNode.toElement();

				// try to get a nice high res photo of our favorite artist
				if(sizeEl.attribute("name") == "original") {
					artist_image_url = sizeEl.text();
					break;
				} 

				// backup plan, extralarge images are rather small though.. 252x250
				if(sizeEl.attribute("name") == "extralarge") {
					artist_image_url = sizeEl.text();
					break;
				}

				// more tiny is useless.

			}

			//m_photoView->requestPhoto ( artist_image_url );
			m_photoView->addPhotoUrl ( QUrl(artist_image_url) );

		}

		if(i == 14) {
		  break; // stop after 5 for now
		}

	}

	m_photoView->start();

}

/**
*
* KeyPress Event checking to enable fullscreen toggle 
*
* F11 can be used to go fullscreen 
*
* To leave fullscreen F11 or ESC can be used
*
*/
void MainWindow::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
    case Qt::Key_F11 :
    {
      if(m_isFullscreen) {
	m_isFullscreen = false;
	statusBar()->setVisible(true);
	showNormal();
      } else {
        m_isFullscreen = true;
        showFullScreen();
	statusBar()->setVisible(false);
      }
    } break;
    case Qt::Key_Escape :
    {
      if(m_isFullscreen) {
	m_isFullscreen = false;
	statusBar()->setVisible(true);
	showNormal();
      }
    }
    break;
  }
}
