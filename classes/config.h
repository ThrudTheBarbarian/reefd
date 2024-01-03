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
	|* Return the directory of the webserver root
	\**********************************************************************/
	QString webDir(void);

	/**********************************************************************\
	|* Return the port to run the server-socket on
	\**********************************************************************/
	int networkPort(void);

	/**********************************************************************\
	|* Return the port to run the server-socket on
	\**********************************************************************/
	int cacheSize(void);

	/**********************************************************************\
	|* Set if we want a clean start, deletes everything
	\**********************************************************************/
	bool reinitialise(void);
	};

#endif // CONFIG_H
