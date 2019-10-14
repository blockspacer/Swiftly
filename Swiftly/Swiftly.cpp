#include "Swiftly.h"
#include "HttpHeader.h"
#include "Worker.h"
#include "IncomingConnectionQueue.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <sodium.h>
#include "ReCAPTCHAVerifier.h"
#include <QSettings>
#include "SettingsManager.h"
#include <QCryptographicHash>
#include <QStringBuilder>

HttpServer::HttpServer(QObject* parent )
    : QTcpServer(parent), 
      m_connectionCount(0),
      m_disabled(false),
      //m_incomingConnectionQueue(nullptr),
      m_webAppSet(),
      m_count(0)

{
    qRegisterMetaType<qintptr>("qintptr");
    //qRegisterMetaType<HttpRequest>("HttpRequest");
    //qRegisterMetaType<HttpResponse>("HttpResponse");
    setMaxPendingConnections(20000);
    //m_incomingConnectionQueue = new IncomingConnectionQueue(this);
}

HttpServer::~HttpServer()
{
}

void HttpServer::incomingConnection(qintptr socket)
{
    qDebug() << "sodium problem";
    if (m_disabled)
        return;

    //m_incomingConnectionQueue->addSocket(socket);
    m_workerPool[m_count]->addNewSocket(socket);

    //qDebug()<<"New Connection!" << serverAddress().toString() << socket;

    m_connectionCount++;
    m_count = (m_count+1)%m_workerPool.size();



    //qDebug() << "connection:" << m_connectionCount;
}

void HttpServer::start(int numOfWorkers, quint16 port)
{
    if(sodium_init()!=0)
    {
        qDebug() << "sodium problem";
    }

    SettingsManager::getSingleton().init();

    if(SettingsManager::getSingleton().has("reCAPTCHA/secret"))
    {
        qDebug() << "has reCAPTCHA settings";
        QString secret = SettingsManager::getSingleton().get("reCAPTCHA/secret").toString();
        ReCAPTCHAVerifier::getSingleton().init(secret);
    }

    QString adminPassHash;
    QString consolePath;

    if (SettingsManager::getSingleton().has("admin/pass") && SettingsManager::getSingleton().has("admin/path"))
    {
        qDebug() << "request admin console";
        adminPassHash = QCryptographicHash::hash(SettingsManager::getSingleton().get("admin/pass").toString().toUtf8(), QCryptographicHash::Sha512).toHex();
        consolePath = SettingsManager::getSingleton().get("admin/path").toString();
    }

    qDebug()<<"Need to create"<<numOfWorkers<<"workers";

    if(numOfWorkers<1)
    {
        numOfWorkers=1;
    }

    for(int i=0;i<numOfWorkers;++i)
    {
        Worker *aWorker=new Worker(QString("worker %1").arg(i), consolePath, adminPassHash);
        aWorker->moveToThread(aWorker);
        aWorker->registerWebApps(m_webAppSet);
        aWorker->start();
        aWorker->setPriority(QThread::HighPriority);
        connect(aWorker, SIGNAL(shutdown()), this, SLOT(shutdown()));
        m_workerPool.push_back(aWorker);
    }

    listen(QHostAddress::Any, port);
    qDebug() << "Port: " << port;

    qDebug()<<"Start listening! main ThreadId"<<thread()->currentThreadId();
}

void HttpServer::shutdown()
{
    close();
    m_disabled = true;

    for(int i =0;i<m_workerPool.size();++i)
    {
        m_workerPool[i]->wait();
    }

    qDebug() << "all worker finished!";
    QCoreApplication::quit();
}

void HttpServer::pause()
{
    m_disabled = true;
}

void HttpServer::resume()
{
    m_disabled = false;
}

void urlParameterParser(const QByteArray &parameters, QHash<QString, QString> &parameterList)
{
    QString parameterString = QString::fromUtf8(parameters);

    QStringList parameterStringList = parameterString.split("&");

    foreach (const QString &str, parameterStringList)
    {
        QStringList parameterPair = str.split("=");

        if (parameterPair.size() == 2)
        {
            parameterList[parameterPair[0]] = parameterPair[1];
        }
        else
        {
            qDebug() << "parameter list parsing error";
        }
    }
}

void generateHashCode(QByteArray &activationCode)
{
    const unsigned int activationCodeSize = 256;
    QByteArray buffer;
    buffer.resize(activationCodeSize);
    randombytes_buf(static_cast<void *>(buffer.data()), activationCodeSize);
    QCryptographicHash hash(QCryptographicHash::Sha3_256);
    hash.addData(buffer);
    activationCode = hash.result().toHex();

    qDebug() << activationCode;
}

void generateRedirectPageContent(QString &content, const QString &url, unsigned int delay)
{
    content = "<html>"
              "<head>"
              "<title>Will redirect after " % QString::number(delay) % " seconds</title>"
              "<meta http-equiv=\"refresh\" content=\"" % QString::number(delay) % "; URL=" % url % "\">"
              "</head>"
              "<body>"
              "If your browser doesn't automatically redirect you to the new address within a few seconds, "
              "you may want to go to "
              "<a href=\"" % url % "\">" % url % "</a> "
              "manually."
              "</body>"
              "</html>";
}
