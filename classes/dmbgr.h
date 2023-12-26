#ifndef DMBGR_H
#define DMBGR_H

#include <QObject>

#include "properties.h"

QT_FORWARD_DECLARE_CLASS(QWebSocket)

class DbMgr : public QObject
	{
	Q_OBJECT

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GET(bool, dbOk);				// Whether the database could open

	private:
		/**********************************************************************\
		|* Upgrade the schema if necessary
		\**********************************************************************/
		void _upgradeDb(void);

		/**********************************************************************\
		|* Create the initial schema for the database
		\**********************************************************************/
		void _createInitialSchema(void);

	public:
		/**********************************************************************\
		|* Constructor / Destructor
		\**********************************************************************/
		explicit DbMgr(QObject *parent = nullptr);
		~DbMgr(void);

	signals:
		/**********************************************************************\
		|* Tell the world we have the JSON ready
		\**********************************************************************/
		void fetchedSystemInfo(QString json, QString identifier);

	public slots:
		/**********************************************************************\
		|* Accept a request to find the groups for a user
		\**********************************************************************/
		void fetchSystemInfo(QString user, QString identifier);

	};

#endif // DMBGR_H
