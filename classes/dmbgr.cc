#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebSocket>
#include <QSqlQuery>

#include "QtCore/qfile.h"
#include "config.h"
#include "constants.h"
#include "dmbgr.h"

#define REEF_DB_ID		"mail"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
Q_LOGGING_CATEGORY(log_db, "reefd:db")

#define LOG qDebug(log_db) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_db) << QTime::currentTime().toString("hh:mm:ss.zzz")


/******************************************************************************\
|* Constructor
\******************************************************************************/
DbMgr::DbMgr(QObject *parent)
	  :QObject{parent}
	{
	/**************************************************************************\
	|* Set up the database for later use
	\**************************************************************************/
	QString dbFile		= Config::instance().databaseDir() + "/reef.db";
	QSqlDatabase db	= QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dbFile);
	_dbOk = db.open();

	if (_dbOk)
		_upgradeDb();
	}


/******************************************************************************\
|* Destructor
\******************************************************************************/
DbMgr::~DbMgr(void)
	{
	QSqlDatabase::database().close();
	}

#pragma mark - private methods

/******************************************************************************\
|* Private method - create the tables in the DB if this is a virgin system and
|*                  upgrade the database if needed
\******************************************************************************/
void DbMgr::_upgradeDb(void)
	{
	/**************************************************************************\
	|* Read the schema version, and see if we need to update it
	\**************************************************************************/
	QSqlQuery query;
	if (query.exec("SELECT version FROM system") == false)
		{
		_createInitialSchema();
		}
	else
		{
		while (query.next())
			{
			int version = query.value(0).toInt();
			switch (version)
				{
				default:
					break;
				}

			// no upgrading yet :)
			break;
			}
		}
	}


/******************************************************************************\
|* Private method - create the initial schema
\******************************************************************************/
void DbMgr::_createInitialSchema(void)
	{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery query;

	if (!query.exec("CREATE TABLE IF NOT EXISTS system\n"
				"(\n"
				"version INTEGER NOT NULL DEFAULT 0\n"
				")\n"))
		ERR <<"Cannot create system table";

	if (!query.exec("INSERT OR REPLACE INTO system (version) VALUES (1)"))
		ERR << "Cannot insert system version";

	if (!query.exec("CREATE TABLE IF NOT EXISTS inputs\n"
					"(\n"
					"id      INTEGER PRIMARY KEY,\n"
					"name    VARCHAR(128) NOT NULL DEFAULT 'unnamed',\n"
					"module  INTEGER NOT NULL DEFAULT -1,\n"
					"driver  VARCHAR(128) NOT NULL DEFAULT 'unknown'\n"
					")\n"))
		ERR << "Cannot create inputs table";

	if (!query.exec("CREATE TABLE IF NOT EXISTS outputs\n"
					"(\n"
					"id      INTEGER PRIMARY KEY,\n"
					"name    VARCHAR(128) NOT NULL DEFAULT 'unnamed',\n"
					"module  INTEGER NOT NULL DEFAULT -1,\n"
					"driver  VARCHAR(128) NOT NULL DEFAULT 'unknown'\n"
					")\n"))
		ERR << "Cannot create outputs table";

	QFile f(Config::instance().databaseDir() + "/reef.sql");
	if (f.open(QFile::ReadOnly))
		{
		QString content = QString::fromUtf8(f.readAll());
		f.close();
		QStringList statements = content.split(";");
		for (QString& statement : statements)
			query.exec(statement);
		}
	}


#pragma mark - slots


/******************************************************************************\
|* Slot: Get the system configuration for a given user's view. Currently all
|*       users get the same view
\******************************************************************************/
void DbMgr::fetchSystemInfo(QString user, QString identifier)
	{
	(void)user;

	QSqlDatabase db = QSqlDatabase::database();

	/**************************************************************************\
	|* Perform the query to return all the folder names. Only deals with
	|* top-level folders right now
	\**************************************************************************/
	QSqlQuery query;
	query.exec("SELECT id, name, module, driver "
				"FROM inputs "
			   "ORDER BY name");

	QJsonArray list;
	QJsonObject record;
	while (query.next())
		{
		record.insert("id", QJsonValue::fromVariant(query.value(0)));
		record.insert("name", QJsonValue::fromVariant(query.value(1)));
		record.insert("module", QJsonValue::fromVariant(query.value(2)));
		record.insert("driver", QJsonValue::fromVariant(query.value(3)));
		list.push_back(record);
		}

	/**************************************************************************\
	|* Create the JSON record and send to the client who called us
	\**************************************************************************/
	QJsonDocument result(list);
	emit fetchedSystemInfo(result.toJson(), identifier);
	}
