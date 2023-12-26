#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QDebug>
#include <QTime>
#include <QLoggingCategory>

/******************************************************************************\
|* Identity
\******************************************************************************/
#define ORG_NAME				"Reef-controller"
#define ORG_DOMAIN				"reef-controller.net"
#define DAEMON_NAME				"reefd"
#define DAEMON_VERSION			"0.1"

/******************************************************************************\
|* Logging
\******************************************************************************/
Q_DECLARE_LOGGING_CATEGORY(log_main)
Q_DECLARE_LOGGING_CATEGORY(log_cfg)

#endif // CONSTANTS_H
