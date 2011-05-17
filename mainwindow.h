#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>
#include <QtXml/QDomDocument>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QtGui/QMainWindow>
#include <QtNetwork/QNetworkReply>
#include <QtWebKit/QWebView>
#include <QtSql>

struct PhotoInfo
{
  QString owner;
  QString title;
  QString description;
  QString location;
  QUrl sourceUrl;
  QString searchString;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void replyFinished(QNetworkReply*);
    void downloadProgress(qint64 bytes, qint64 bytesTotal);
    QString tempStorageDir() const { return _tempStorageDir; }

private:
    QWebView * wv;
    QNetworkAccessManager *manager;
    QNetworkReply * reply;
    QProgressBar * progress;
    void readLastFM(const QDomDocument& doc) const;

    QSqlDatabase db;
    void initializeDB();
    void updateDBVersion();
    void updateDB();
    void cleanHistory(int now, int timeBack, int timeBackFactor);
    void clearHistory();

    QString _tempStorageDir;

    QSize currentPhotoSize;
    QFileInfo currentFile;
    PhotoInfo currentPhotoInfo;

};

#endif // MAINWINDOW_H
