#include <QCoreApplication>
#include <QtCore/QThread>
#include "Swiftly.h"
#include "HelloWorld.h"

int main(int argc, char *argv[])
{
    qDebug() << "Example HelloWorld";
    QCoreApplication::setOrganizationName("Swiftly");
    QCoreApplication::setOrganizationDomain("Swiftly.com");
    QCoreApplication::setApplicationName("HelloWorld example");
    QCoreApplication a(argc, argv);
    REGISTER_WEBAPP(HelloWorld);
    HttpServer::getSingleton().start(QThread::idealThreadCount(), 8080);
    return a.exec();
}
