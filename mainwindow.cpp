#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QStatusBar>
#include <QProgressBar>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>
#include "photo.h"

MainWindow::MainWindow(QString &artist, QWidget *parent) : QMainWindow(parent)
{

    prepareHomeDir();

    /* Progress bar to signal progress of RSS feed download and web page load. */
    progress = new QProgressBar;
    statusBar()->addPermanentWidget(progress);

    photoView = new PhotoView();
// TODO: reimplement this getmore function 
//    connect ( photoView,SIGNAL ( giveMeMore(int) ),this,SLOT ( getPhotos(int) ) );
    this->setCentralWidget ( photoView );

    resize(800,600);

//    getPhotos(MAX_PHOTOS);

    /* Set up the network manager. */
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    artist = (artist == "") ? "Jimi Hendrix" : artist;

    fetchPhotos(artist);
}

MainWindow::~MainWindow()
{
}

void MainWindow::prepareHomeDir()
{

#ifdef Q_WS_X11
  QDir(QDir::homePath()).mkdir(TEMP_PATH);
  _tempStorageDir = QDir::homePath() + "/" + TEMP_PATH;
#else
  QDir::home().root().mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
  _tempStorageDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif

}

void MainWindow::downloadProgress(qint64 bytes, qint64 bytesTotal)
{
    if (bytesTotal == -1) {
        /* No total bytes available, just set the progress bar to show a busy indicator. */
        progress->setMinimum(0);
        progress->setMaximum(0);
    } else {
        progress->setMaximum(100);
        int percent = bytes * 100 / bytesTotal;
        progress->setValue(percent);
    }
}

void MainWindow::fetchPhotos(const QString &artist) {

    // check DB for photos

    // otherwise get it from lastFM
    QString rss_url = "http://ws.audioscrobbler.com/2.0/?method=artist.getImages&api_key=2978eaa78e3d2cc0e6033ec16ac41395&artist=" + artist;

    reply = manager->get(QNetworkRequest(QUrl(rss_url)));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));

}

void MainWindow::replyFinished(QNetworkReply * netReply)
{
    QString str (netReply->readAll());

    /* If we are redirected, try again. TODO: Limit redirection count. */
    QVariant vt = netReply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    delete reply;

    if (!vt.isNull()) {
        qDebug() << "Redirected to:" << vt.toUrl().toString();
        reply = manager->get(QNetworkRequest(vt.toUrl()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    } else {
        /* We have something. */
        QDomDocument doc;
        QString error;
        if (!doc.setContent(str, false, &error)) {
		// report an error somewhere..
            //wv->setHtml(QString("<h1>Error</h1>") + error);
		qDebug() << error; 
        } else {

	    // check the db first
	    readLastFM(doc);
        }
    }
}

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
			qDebug() << "No photos found";
		} else {
			qDebug() << "Found some photo's";
			for (uint j = 0; j < sizeList.length(); j++) {

				QDomNode sizeNode  = sizeList.item(j);
				QDomElement sizeEl = sizeNode.toElement();

				qDebug() << sizeEl.attribute("name");

				// try to get a nice high res photo of our favorite artist
				if(sizeEl.attribute("name") == "original") {
					qDebug() << "Found an highres original! yeay";
					artist_image_url = sizeEl.text();
					break;
				} 

				// backup plan, extralarge images are rather small though.. 252x250
				if(sizeEl.attribute("name") == "extralarge") {
					artist_image_url = sizeEl.text();
					break;
				}

			}

			// more tiny is useless.
			photoView->addPhoto ( artist_image_url );

			qDebug() << "photoView->addPhoto: " << artist_image_url;
		}

		if(i == 14) {
		  break; // stop after 5 for now
		}

	}

}

void MainWindow::initializeDB()
{
  qDebug() << "Application::initializeDB()";

  db = QSqlDatabase::addDatabase("QSQLITE");
  QDir(_tempStorageDir).mkdir("thumbs");
  db.setDatabaseName(_tempStorageDir + "/thumbs/LastFmSlideshow.db");
  if (db.open()) {
    QSqlQuery query(db);

    query.exec("PRAGMA journal_mode = OFF");

    if (query.exec("select * from sqlite_master where name = 'photoHistory'"))
    {

      QString createTable = "create table photoHistory (timestamp integer primary key, url text, artist text, title text, description text, location text, sourceUrl text, width integer,height integer)";

      if (!query.next() && (!query.exec(createTable))) {
        db.close();
        qDebug() << "Application::Application() cannot create photoHistory table";
      }
    }
  } else {
    qDebug() << "Application::Application() db connection failed";
  }
}

void MainWindow::updateDB()
{

  QByteArray cTags;
  int now = QDateTime::currentDateTime().toTime_t();

  qDebug() << "Application::updateDB()";

  // TODO: cleanHistory reimplement later ( or not )
  //cleanHistory(now,settings.value(MAIN_SECTION).value(HISTORY_TIME_LIMIT).toInt(), settings.value(MAIN_SECTION).value(HISTORY_TIME_LIMIT_FACTOR).toInt());

  if (currentPhotoInfo.searchString.isEmpty()) { return; }

  // TODO: reimplement history limit, in our case photoUrl is never empty at this stage
  //if (db.isOpen() && !engines.at(currentEngineIndex)->photoUrl().isEmpty() && settings.value(MAIN_SECTION).value(HISTORY_TIME_LIMIT).toInt())
  if (db.isOpen()) {

    QSqlQuery query(db);

    QString insert = "insert into photoHistory (timestamp, url, artist, title, description, location, sourceUrl, width, height) values (:timestamp, :url, :title, :description, :location, :sourceUrl, :width, :height)";

    query.prepare(insert);

    query.bindValue(":timestamp",now);
    query.bindValue(":url", ""); // if needed pass it through currentPhotoInfo or something
    query.bindValue(":artist",currentPhotoInfo.searchString.toLower());
    query.bindValue(":title",currentPhotoInfo.title);
    query.bindValue(":owner",currentPhotoInfo.owner);
    query.bindValue(":description",currentPhotoInfo.description);
    query.bindValue(":location",currentPhotoInfo.location);
    query.bindValue(":sourceUrl",currentPhotoInfo.sourceUrl.toString());
    query.bindValue(":size",currentFile.size());
    query.bindValue(":width",currentPhotoSize.width());
    query.bindValue(":height",currentPhotoSize.height());

    if (query.exec()) {
      QImage thumb(currentFile.absoluteFilePath());

      if (thumb.width() > thumb.height()) {
	      thumb = thumb.scaledToWidth(100,Qt::SmoothTransformation);
      } else {
	      thumb = thumb.scaledToHeight(100,Qt::SmoothTransformation);
      }

      // TODO: create dirs based on artist name
      thumb.save(_tempStorageDir + "/thumbs/" + QString::number(now) + ".png","png");
    } else {
      qDebug() << "Application::updateDB() insert error:" << query.lastError().text();
    }
  }
}

void MainWindow::clearHistory()
{
  QSqlQuery query(db);
  QDirIterator dirIterator(_tempStorageDir + "/thumbs",
                           QStringList() << "*.png",
                           QDir::Files);

  qDebug() << "Application::clearHistory()";

  if (!query.exec("delete from photoHistory") || !query.exec("vacuum")) {
    qDebug() << "Application::clearHistory() delete error:" << query.lastError().text();
  }

  while (dirIterator.hasNext()) {
    dirIterator.next();
    QFile::remove(dirIterator.filePath());
  }
}

