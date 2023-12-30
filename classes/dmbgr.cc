#include <QFile>
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
	QString dbFile = Config::instance().databaseDir() + "/reef.db";

	/**************************************************************************\
	|* If we're told to re-initialise then just delete any old database
	\**************************************************************************/
	if (Config::instance().reinitialise())
		{
		QFile f(dbFile);
		if (f.exists())
			if (!f.remove())
				ERR << "Cannot remove database file " << dbFile;
		}

	/**************************************************************************\
	|* Set up the database for later use
	\**************************************************************************/
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
					"driver  VARCHAR(128) NOT NULL DEFAULT 'unknown',\n"
					"render  VARCHAR(128) NOT NULL DEFAULT 'unknown'\n"
					")\n"))
		ERR << "Cannot create inputs table";

	if (!query.exec("CREATE TABLE IF NOT EXISTS outputs\n"
					"(\n"
					"id      INTEGER PRIMARY KEY,\n"
					"name    VARCHAR(128) NOT NULL DEFAULT 'unnamed',\n"
					"module  INTEGER NOT NULL DEFAULT -1,\n"
					"driver  VARCHAR(128) NOT NULL DEFAULT 'unknown',\n"
					"render  VARCHAR(128) NOT NULL DEFAULT 'unknown'\n"
					")\n"))
		ERR << "Cannot create outputs table";

	if (!query.exec("CREATE TABLE IF NOT EXISTS modules\n"
					"(\n"
					"id      INTEGER PRIMARY KEY,\n"
					"name    VARCHAR(128) NOT NULL DEFAULT 'unnamed',\n"
					"driver  VARCHAR(128) NOT NULL DEFAULT 'unknown',\n"
					"nodeId  VARCHAR(32) NOT NULL DEFAULT '',\n"
					"render  VARCHAR(128) NOT NULL DEFAULT 'unknown'\n"
					")\n"))
		ERR << "Cannot create modules table";

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
	|* Add the list of known modules to the results
	\**************************************************************************/
	QSqlQuery query;
	query.exec("SELECT id, name, nodeId, driver, render "
			   "FROM modules "
			   "ORDER BY name");

	QJsonObject records;
	QJsonArray modules;
	QJsonObject module;
	while (query.next())
		{
		module.insert("id", QJsonValue::fromVariant(query.value(0)));
		module.insert("name", QJsonValue::fromVariant(query.value(1)));
		module.insert("nodeId", QJsonValue::fromVariant(query.value(2)));
		module.insert("driver", QJsonValue::fromVariant(query.value(3)));
		module.insert("renderer", QJsonValue::fromVariant(query.value(4)));
		modules.push_back(module);
		}
	records.insert("modules", modules);

	/**************************************************************************\
	|* Add the list of known inputs to the results
	\**************************************************************************/
	query.exec("SELECT id, name, module, driver, render "
			   "FROM inputs "
			   "ORDER BY name");

	QJsonArray inputs;
	QJsonObject input;
	while (query.next())
		{
		input.insert("id", QJsonValue::fromVariant(query.value(0)));
		input.insert("name", QJsonValue::fromVariant(query.value(1)));
		input.insert("module", QJsonValue::fromVariant(query.value(2)));
		input.insert("driver", QJsonValue::fromVariant(query.value(3)));
		input.insert("renderer", QJsonValue::fromVariant(query.value(4)));
		inputs.push_back(input);
		}
	records.insert("inputs", inputs);

	/**************************************************************************\
	|* Add the list of known outputs to the results
	\**************************************************************************/
	query.exec("SELECT id, name, module, driver, render "
			   "FROM outputs "
			   "ORDER BY name");

	QJsonArray outputs;
	QJsonObject output;
	while (query.next())
		{
		output.insert("id", QJsonValue::fromVariant(query.value(0)));
		output.insert("name", QJsonValue::fromVariant(query.value(1)));
		output.insert("module", QJsonValue::fromVariant(query.value(2)));
		output.insert("driver", QJsonValue::fromVariant(query.value(3)));
		output.insert("renderer", QJsonValue::fromVariant(query.value(4)));
		outputs.push_back(output);
		}
	records.insert("outputs", outputs);

	/**************************************************************************\
	|* Add the name of this return-type
	\**************************************************************************/
	QJsonValue methodName = "SysInfo";
	records.insert("method", methodName);

	/**************************************************************************\
	|* Create the JSON record and send to the client who called us
	\**************************************************************************/
	QJsonDocument result(records);
	emit fetchedSystemInfo(result.toJson(), identifier);
	}
