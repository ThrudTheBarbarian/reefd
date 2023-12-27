#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

#include "config.h"
#include "constants.h"


/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_cfg) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_cfg) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* These are the keys we look for in the config file
\******************************************************************************/
#define SYSTEM_GROUP			"system"
#define SYSTEM_DATA_DIR_KEY		"data-dir"
#define SYSTEM_DATA_DIR_DFLT	"/Volumes/raid/reefd"
#define SYSTEM_INIT_KEY			"re-initialise"
#define SYSTEM_INIT_DFLT		"0"


#define NETWORK_GROUP			"network"
#define NETWORK_PORT_KEY		"network-port"
#define NETWORK_PORT_DFLT		"5417"

#define DECODE(x,k,dflt) (x.value(k,dflt).toString())

/******************************************************************************\
|* These are the commandline args we're managing
\******************************************************************************/
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
						  _help,
						  ({"h", "help"}, "Show this useful (?) help"))

Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
						  _reInit,
						  ({"i", "initialise"},
						   "Initialise everything at boot. Dangerous."))


Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
						  _dataDir,
						  ({"d", SYSTEM_DATA_DIR_KEY},
						   "Directory to store data",
						   SYSTEM_DATA_DIR_DFLT))

Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
						  _networkPort,
						  ({"p", NETWORK_PORT_KEY},
						   "Network socket port number",
						   NETWORK_PORT_DFLT))

Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
						  _version,
						  ({"v", "version"},
						   "Display the daemon version"))

Config::Config()
	{
	/**************************************************************************\
	|* Configure the parser
	\**************************************************************************/
	_parser.setApplicationDescription("Mail daemon");
	_parser.addOption(*_dataDir);
	_parser.addOption(*_help);
	_parser.addOption(*_reInit);
	_parser.addOption(*_networkPort);
	_parser.addOption(*_version);


	/**************************************************************************\
	|* Parse and check for help / version options
	\**************************************************************************/
	_parser.parse(QCoreApplication::arguments());
	if (_parser.isSet(*_help))
		{
		QString help = _parser.helpText();
		fprintf(stderr, "%s\n\n"
				,qUtf8Printable(help)
				);
		exit(0);
		}

	if (_parser.isSet(*_version))
		_parser.showVersion();
	}

/******************************************************************************\
|* Get where we read user databases from
\******************************************************************************/
QString Config::databaseDir(void)
	{
	if (_parser.isSet(*_dataDir))
		return _parser.value(*_dataDir);

	QSettings s;
	s.beginGroup(SYSTEM_GROUP);
	QString dir = DECODE(s, SYSTEM_DATA_DIR_KEY, SYSTEM_DATA_DIR_DFLT);
	s.endGroup();
	return dir;
	}

/******************************************************************************\
|* Get the port number to operate the server-socket on
\******************************************************************************/
int Config::networkPort(void)
	{
	if (_parser.isSet(*_networkPort))
		return _parser.value(*_networkPort).toInt();

	QSettings s;
	s.beginGroup(NETWORK_GROUP);
	QString port = DECODE(s, NETWORK_PORT_KEY, NETWORK_PORT_DFLT);
	s.endGroup();
	return port.toInt();
	}

/******************************************************************************\
|* Determine if we should reset to factory defaults
\******************************************************************************/
bool Config::reinitialise(void)
	{
	if (_parser.isSet(*_reInit))
		return _parser.value(*_reInit).toInt() != 0;

	QSettings s;
	s.beginGroup(SYSTEM_GROUP);
	QString init = DECODE(s, SYSTEM_INIT_KEY, SYSTEM_INIT_DFLT);
	s.endGroup();
	return init.toInt() != 0;
	}

