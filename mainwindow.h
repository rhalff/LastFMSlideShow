#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>
#include <QtXml/QDomDocument>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QtGui/QMainWindow>
#include <QtNetwork/QNetworkReply>
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

class PhotoView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString &artist, QWidget* parent = 0);
    ~MainWindow();

public slots:
    void replyFinished(QNetworkReply*);
    void downloadProgress(qint64 bytes, qint64 bytesTotal);
    QString tempStorageDir() const { return m_tempStorageDir; }

protected:
    void keyPressEvent(QKeyEvent*);

private:
    QNetworkAccessManager *manager;
    QNetworkReply * m_reply;
    QProgressBar * m_progress;
    QString m_tempStorageDir;
    QSize m_currentPhotoSize;
    QFileInfo m_currentFile;
    PhotoInfo m_currentPhotoInfo;
    PhotoView * m_photoView;

    int m_deskX;
    int m_deskY;
    int m_deskWidth;
    int m_deskHeight;
    bool m_isFullscreen;

    void readLastFM(const QDomDocument& doc) const;

    QSqlDatabase db;
    void initializeDB();
    void updateDBVersion();
    void updateDB();
    void cleanHistory(int now, int timeBack, int timeBackFactor);
    void clearHistory();
    void fetchPhotos(const QString &artist);
    void prepareHomeDir();
    void resizeScreen();

};

#endif // MAINWINDOW_H
