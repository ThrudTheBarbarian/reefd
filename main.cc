#include <QCoreApplication>
#include <QThread>
#include <QWebSocket>

#include "config.h"
#include "constants.h"
#include "dmbgr.h"
#include "socket.h"

#define CONNECT		QObject::connect

int main(int argc, char *argv[])
	{
	QCoreApplication a(argc, argv);

	/**************************************************************************\
	|* Set up the settings for the application
	\**************************************************************************/
	QCoreApplication::setOrganizationName(ORG_NAME);
	QCoreApplication::setOrganizationDomain(ORG_DOMAIN);
	QCoreApplication::setApplicationName(DAEMON_NAME);
	QCoreApplication::setApplicationVersion(DAEMON_VERSION);


	/**************************************************************************\
	|* Set up the configuration from both settings and commandline
	\**************************************************************************/
	Config &cfg = Config::instance();

	/**************************************************************************\
	|* Configure the message i/o handler (websocket-based)
	\**************************************************************************/
	QThread networkThread;
	Socket ws;
	ws.init(cfg.networkPort());

	ws.moveToThread(&networkThread);
	networkThread.start();

	/**************************************************************************\
	|* Create a database context to handle user storage and metadata
	\**************************************************************************/
	DbMgr db;

	/**************************************************************************\
	|* Connect up the query/response for the system-info
	\**************************************************************************/
	CONNECT(&ws, &Socket::fetchSystemInfo, &db, &DbMgr::fetchSystemInfo);
	CONNECT(&db, &DbMgr::fetchedSystemInfo, &ws, &Socket::sendSystemInfo);

	return a.exec();
	}
