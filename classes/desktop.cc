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
	}



#pragma mark - slots

/******************************************************************************\
|* Slot: Get a list of all the icons available to the web-app on the desktop
\******************************************************************************/
void Desktop::fetchDesktopIcons(QString user, QString identifier)
	{
	(void)user;

	QStringList filters;
	filters << "*.png" ;
	QDir dir(_rsrcDir + ICONS_PATH);
	QStringList iconList = dir.entryList(filters);

	QJsonObject records;
	QJsonArray icons;
	for (QString& iconName : iconList)
		{
		QStringList parts = iconName.split(".");
		if (parts.length() > 0)
			{
			QJsonObject icon;
			icon.insert("name", parts.at(0));
			icon.insert("path", QString("/") + ICONS_PATH + iconName);
			icon.insert("driver", parts.at(0) + "IconDriver");
			icons.append(icon);
			}
		}

	/**************************************************************************\
	|* Create the JSON record and send to the client who called us
	\**************************************************************************/
	records.insert("icons", icons);
	records.insert("method", "DesktopIcons");

	QJsonDocument result(records);
	emit fetchedDesktopIcons(result.toJson(), identifier);
	}
