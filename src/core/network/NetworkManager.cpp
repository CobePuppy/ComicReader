#include "../../include/core/NetworkManager.h"
#include <QNetworkProxy>
#include <QDebug>

NetworkManager* NetworkManager::m_instance = nullptr;

NetworkManager* NetworkManager::instance()
{
    if (!m_instance) {
        m_instance = new NetworkManager();
    }
    return m_instance;
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_userAgent("ComicReader/1.0")
    , m_timeout(30000) // 30秒超时
    , m_timeoutTimer(new QTimer(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &NetworkManager::onReplyFinished);
    
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &NetworkManager::onTimeout);
}

NetworkManager::~NetworkManager()
{
    // 取消所有活动请求
    for (QNetworkReply *reply : m_activeRequests) {
        reply->abort();
        reply->deleteLater();
    }
    m_activeRequests.clear();
}

QNetworkReply* NetworkManager::get(const QUrl &url, const QNetworkRequest &request)
{
    QNetworkRequest req = request;
    req.setUrl(url);
    setupRequest(req);
    
    QNetworkReply *reply = m_networkManager->get(req);
    m_activeRequests.enqueue(reply);
    
    connect(reply, &QNetworkReply::downloadProgress,
            this, &NetworkManager::onDownloadProgress);
    
    // 启动超时计时器
    m_timeoutTimer->start(m_timeout);
    
    return reply;
}

QNetworkReply* NetworkManager::post(const QUrl &url, const QByteArray &data, const QNetworkRequest &request)
{
    QNetworkRequest req = request;
    req.setUrl(url);
    setupRequest(req);
    
    QNetworkReply *reply = m_networkManager->post(req, data);
    m_activeRequests.enqueue(reply);
    
    connect(reply, &QNetworkReply::downloadProgress,
            this, &NetworkManager::onDownloadProgress);
    
    // 启动超时计时器
    m_timeoutTimer->start(m_timeout);
    
    return reply;
}

void NetworkManager::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
}

void NetworkManager::setTimeout(int timeoutMs)
{
    m_timeout = timeoutMs;
}

void NetworkManager::setProxy(const QString &host, int port)
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName(host);
    proxy.setPort(port);
    m_networkManager->setProxy(proxy);
}

void NetworkManager::clearProxy()
{
    m_networkManager->setProxy(QNetworkProxy::NoProxy);
}

void NetworkManager::setupRequest(QNetworkRequest &request)
{
    // 设置用户代理
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());
    
    // 设置其他常用头部
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Connection", "keep-alive");
    
    // 设置SSL配置
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);
}

void NetworkManager::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        m_activeRequests.removeAll(reply);
        
        // 停止超时计时器
        if (m_activeRequests.isEmpty()) {
            m_timeoutTimer->stop();
        }
        
        emit requestFinished(reply);
        reply->deleteLater();
    }
}

void NetworkManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void NetworkManager::onTimeout()
{
    qDebug() << "Network request timeout";
    
    // 中止所有活动请求
    for (QNetworkReply *reply : m_activeRequests) {
        reply->abort();
    }
}
