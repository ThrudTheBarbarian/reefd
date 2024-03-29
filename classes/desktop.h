#ifndef DESKTOP_H
#define DESKTOP_H

#include <QObject>

#include "properties.h"

class Desktop : public QObject
	{
	Q_OBJECT

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GET(QString, rsrcDir);				// Location of resources
	GET(QString, fmwkDir);				// Location of frameworks

	private:
		/**********************************************************************\
		|* Method: find all plugins of a given type
		\**********************************************************************/
		void _findPlugins(QJsonObject& records, QString filter, QString type);

	public:
		/**********************************************************************\
		|* Constructor / Destructor
		\**********************************************************************/
		explicit Desktop(QObject *parent = nullptr);

	signals:
		/**********************************************************************\
		|* Tell the world we have the JSON ready
		\**********************************************************************/
		void fetchedDesktopIcons(QString json, QString identifier);
		void fetchedDesktopApps(QString json, QString identifier);

	public slots:
		/**********************************************************************\
		|* Accept a request to find the groups for a user
		\**********************************************************************/
		void fetchDesktopIcons(QString user, QString identifier);

		/**********************************************************************\
		|* Accept a request to find the apps for a user
		\**********************************************************************/
		void fetchDesktopApps(QString user, QString identifier);
	};

#endif // DESKTOP_H
