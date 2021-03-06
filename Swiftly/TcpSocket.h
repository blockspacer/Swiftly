#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>
#include <QByteArray>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <QTimer>

class TcpSocket:public QTcpSocket
{
    Q_OBJECT

    bool m_isNew;

    HttpRequest m_request;
    HttpResponse m_response;
    QTimer m_suicideTimer;

public:
    unsigned int m_id;
    TcpSocket(QObject *parent = nullptr);
    TcpSocket(const TcpSocket &in);
    void operator=(const TcpSocket &in);
    ~TcpSocket();

    void setRawHeader(const QString &in);
    QString & getRawHeader();

    unsigned int getTotalBytes();
    unsigned int getBytesHaveRead();
    HttpHeader & getHeader();
    bool isEof();
    void notNew();

    bool isNewSocket();
    void setTotalBytes(unsigned int _totalBytes);

    void appendData(const char* buffer,unsigned int size);
    void appendData(const QByteArray &buffer);

    HttpRequest & getRequest()
    {
        return m_request;
    }

    HttpResponse & getResponse()
    {
        return m_response;
    }

    void setTimeout(int msec);

private slots:
    void disconnectSocket();
};

#endif // TCPSOCKET_H
