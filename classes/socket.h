#ifndef SOCKET_H
#define SOCKET_H

#include <QMap>
#include <QMutex>
#include <QObject>

#include "properties.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class Socket : public QObject
	{
	Q_OBJECT

	/**************************************************************************\
	|* Properties
	\**************************************************************************/

	public:
	/**********************************************************************\
	|* Typedefs and enums
	\**********************************************************************/

	private:
	/**********************************************************************\
	|* Private variables
	\**********************************************************************/
	QWebSocketServer *			_server;		// Handle the connection
	QMap<QString,QWebSocket *>	_clients;		// Map of connected clients
	QMutex						_lock;			// Thread safety


	private slots:
	/**********************************************************************\
	|* Private slots - generally for WebSocket operation
	\**********************************************************************/
	void onNewConnection(void);
	void socketDisconnected();
	void processTextMessage(const QString &message);
	void processBinaryMessage(QByteArray message);


	public:
	/**********************************************************************\
	|* Constructor / Destructor
	\**********************************************************************/
	explicit Socket(QObject *parent = nullptr);
	~Socket() override;

	/**********************************************************************\
	|* Initialise the server
	\**********************************************************************/
	void init(int port);

	/**********************************************************************\
	|* Send appropriate message types
	\**********************************************************************/
	void sendText(const QString &message, QString identifier = "");
	void sendData(const QByteArray &data, QString identifier = "");

	signals:

	/**********************************************************************\
	|* Call out to the database to fetch the current setup, as a given user
	\**********************************************************************/
	void fetchSystemInfo(QString user, QString identifier);

	/**********************************************************************\
	|* We got a disconnection
	\**********************************************************************/
	void disconnection(QString identifier);

	public slots:
	/**********************************************************************\
	|* Send the group info back to the caller
	\**********************************************************************/
	void sendSystemInfo(QString json, QString identifier);
	};

#endif // SOCKET_H
