#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "config.h"
#include "constants.h"
#include "desktop.h"

#define ICONS_PATH		"icons/desktop/"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
Q_LOGGING_CATEGORY(log_desk, "reefd:desktop")

#define LOG qDebug(log_desk) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_desk) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
Desktop::Desktop(QObject *parent)
		:QObject{parent}
	{
	_rsrcDir = Config::instance().webDir() + "/Resources/";
	_fmwkDir = Config::instance().webDir() + "/Frameworks/";
	}



#pragma mark - Private methods

/******************************************************************************\
|* Private method: return all plugins of a given type
\******************************************************************************/
void Desktop::_findPlugins(QJsonObject& records, QString filter, QString type)
	{
	QStringList filters;
	filters << filter ;
	QDir dir(_fmwkDir);
	QStringList list = dir.entryList(filters);

	QJsonArray items;
	for (QString& itemName : list)
		{
		QStringList parts = itemName.split(".");
		if (parts.length() > 0)
			{
			QJsonObject item;
			item.insert("name", parts.at(0));
			item.insert("notify", parts.at(0) + type);
			items.append(item);
			}
		}
	records.insert("plugins", items);
	}



#pragma mark - slots

/******************************************************************************\
|* Slot: Get a list of all the icons available to the web-app on the desktop
\******************************************************************************/
void Desktop::fetchDesktopIcons(QString user, QString identifier)
	{
	(void)user;

	/**************************************************************************\
	|* Fetch all the ".icon" plugins
	\**************************************************************************/
	QJsonObject records;
	_findPlugins(records, "*.icon", "Icon");

	/**************************************************************************\
	|* Create the JSON record and send to the client who called us
	\**************************************************************************/
	records.insert("method", "DesktopIcons");

	QJsonDocument result(records);
	emit fetchedDesktopIcons(result.toJson(), identifier);
	}

/******************************************************************************\
|* Slot: Get a list of all the app frameworks available to the desktop
\******************************************************************************/
void Desktop::fetchDesktopApps(QString user, QString identifier)
	{
	(void)user;

	/**************************************************************************\
	|* Fetch all the ".app" plugins
	\**************************************************************************/
	QJsonObject records;
	_findPlugins(records, "*.app", "App");

	/**************************************************************************\
	|* Create the JSON record and send to the client who called us
	\**************************************************************************/
	records.insert("method", "DesktopApps");

	QJsonDocument result(records);
	emit fetchedDesktopApps(result.toJson(), identifier);
	}
