#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QQueue>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static NetworkManager* instance();
    
    // HTTP请求方法
    QNetworkReply* get(const QUrl &url, const QNetworkRequest &request = QNetworkRequest());
    QNetworkReply* post(const QUrl &url, const QByteArray &data, const QNetworkRequest &request = QNetworkRequest());
    
    // 设置用户代理
    void setUserAgent(const QString &userAgent);
    
    // 设置超时时间
    void setTimeout(int timeoutMs);
    
    // 设置代理
    void setProxy(const QString &host, int port);
    void clearProxy();

signals:
    void requestFinished(QNetworkReply *reply);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private slots:
    void onReplyFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onTimeout();

private:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();
    
    void setupRequest(QNetworkRequest &request);

    static NetworkManager *m_instance;
    QNetworkAccessManager *m_networkManager;
    QString m_userAgent;
    int m_timeout;
    QTimer *m_timeoutTimer;
    QQueue<QNetworkReply*> m_activeRequests;
};

#endif // NETWORKMANAGER_H
