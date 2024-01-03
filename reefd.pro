QT = core
QT += websockets sql

CONFIG += c++17 cmdline sdk_no_version_check

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        classes/config.cc \
        classes/desktop.cc \
        classes/dmbgr.cc \
        classes/socket.cc \
        main.cc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += \
			classes \
			include \

HEADERS += \
	classes/config.h \
	classes/desktop.h \
	classes/dmbgr.h \
	classes/socket.h \
	include/constants.h \
	include/properties.h \
	include/singleton.h
