#ifndef CONFIG_H
#define CONFIG_H

#include <QCommandLineParser>

#include "singleton.h"

class Config : public Singleton<Config>
	{
	private:
	QCommandLineParser		_parser;	// The actual commandline parser

	public:
	/**********************************************************************\
	|* Constructor
	\**********************************************************************/
	explicit Config();

	/**********************************************************************\
	|* Return the database directory for users
	\**********************************************************************/
	QString databaseDir(void);

	/**********************************************************************\
	|* Return the port to run the server-socket on
	\**********************************************************************/
	int networkPort(void);

	/**********************************************************************\
	|* Return the port to run the server-socket on
	\**********************************************************************/
	int cacheSize(void);
	};

#endif // CONFIG_H
