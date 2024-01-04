#include <QtWebSockets>
#include <QWebSocketServer>

#include "socket.h"

/******************************************************************************\
|* Messages
\******************************************************************************/
#define MSG_SYSINFO				"SysInfo"
#define MSG_DESKTOP_ICONS		"DesktopIcons"
#define MSG_DESKTOP_APPS		"DesktopApps"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
Q_LOGGING_CATEGORY(log_net, "reefd   ")

#define LOG qDebug(log_net) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_net) << QTime::currentTime().toString("hh:mm:ss.zzz")


/******************************************************************************\
|* Helper function: Create an identifier for a connection
\******************************************************************************/
static QString getIdentifier(QWebSocket *peer)
	{
	return QStringLiteral("%1:%2").arg(peer->peerAddress().toString(),
									   QString::number(peer->peerPort()));
	}

/******************************************************************************\
|* Constructor
\******************************************************************************/
Socket::Socket(QObject *parent)
	:QObject(parent)
	{

	}

/******************************************************************************\
|* Destructor
\******************************************************************************/
Socket::~Socket(void)
	{
	_server->close();
	}

/******************************************************************************\
|* Initialise
\******************************************************************************/
void Socket::init(int port)
	{
	_server = new QWebSocketServer(QStringLiteral("Maild"),
								   QWebSocketServer::NonSecureMode,
								   this);
	if (_server->listen(QHostAddress::Any, port))
		{
		LOG << "Starting network transport on port" << port;
		connect(_server, &QWebSocketServer::newConnection,
				this, &Socket::onNewConnection);
		}
	else
		ERR << "Cannot start network transport on port" << port;
	}

/******************************************************************************\
|* Handle a client connecting
\******************************************************************************/
void Socket::onNewConnection(void)
	{
	auto socket = _server->nextPendingConnection();
	QString identifier = getIdentifier(socket);

	LOG << "New connection: " << identifier;

	socket->setParent(this);

	connect(socket, &QWebSocket::textMessageReceived,
			this, &Socket::processTextMessage);
	connect(socket, &QWebSocket::disconnected,
			this, &Socket::socketDisconnected);
	connect(socket, &QWebSocket::binaryMessageReceived,
			this, &Socket::processBinaryMessage);

	_clients[identifier] = socket;
	}

/******************************************************************************\
|* Handle a client command message
\******************************************************************************/
void Socket::processTextMessage(const QString& msg)
	{
	QWebSocket *client = qobject_cast<QWebSocket *>(sender());

	if (msg.startsWith(MSG_SYSINFO))
		emit fetchSystemInfo(msg.mid(8).trimmed(), getIdentifier(client));

	else if (msg.startsWith(MSG_DESKTOP_ICONS))
		emit fetchDesktopIcons(msg.mid(12).trimmed(), getIdentifier(client));

	else if (msg.startsWith(MSG_DESKTOP_APPS))
		emit fetchDesktopApps(msg.mid(11).trimmed(), getIdentifier(client));

	else
		LOG << "Unknown message " << msg;
	}

/******************************************************************************\
|* Handle a client binary message
\******************************************************************************/
void Socket::processBinaryMessage(QByteArray msg)
	{
	LOG << "WebSocket got binary. Length : " << msg.length();
	}

/******************************************************************************\
|* Client disconnected
\******************************************************************************/
void Socket::socketDisconnected(void)
	{
	QWebSocket *client = qobject_cast<QWebSocket *>(sender());
	QString identifier = getIdentifier(client);

	if (client)
		{
		emit disconnection(identifier);
		LOG << "Disconnection: " << identifier;

		_clients.remove(identifier);
		client->deleteLater();
		}
	}

/******************************************************************************\
|* Send a text message
\******************************************************************************/
void Socket::sendText(const QString &message, QString identifier)
	{
	if (identifier.length() == 0)
		for (QWebSocket *endpoint : qAsConst(_clients))
			endpoint->sendTextMessage(message);
	else
		{
		QWebSocket * client = _clients[identifier];
		if (client != nullptr)
			client->sendTextMessage(message);
		else
			ERR << "Cannot find client[text] for identifier " << identifier;
		}
	}

/******************************************************************************\
|* Send a binary message
\******************************************************************************/
void Socket::sendData(const QByteArray &data, QString identifier)
	{
	if (identifier.length() == 0)
		for (QWebSocket *endpoint : qAsConst(_clients))
			endpoint->sendBinaryMessage(data);
	else
		{
		QWebSocket * client = _clients[identifier];
		if (client != nullptr)
			client->sendBinaryMessage(data);
		else
			ERR << "Cannot find client[data] for identifier " << identifier;
		}
	}


/******************************************************************************\
|* Slot: Send a system-info message to a specific client
\******************************************************************************/
void Socket::sendSystemInfo(QString json, QString identifier)
	{
	sendText(json, identifier);
	}

/******************************************************************************\
|* Slot: Send a desktop-icons message to a specific client
\******************************************************************************/
void Socket::sendDesktopIcons(QString json, QString identifier)
	{
	sendText(json, identifier);
	}

/******************************************************************************\
|* Slot: Send a desktop-apps message to a specific client
\******************************************************************************/
void Socket::sendDesktopApps(QString json, QString identifier)
	{
	sendText(json, identifier);
	}
