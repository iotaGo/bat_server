TEMPLATE 	= 	app
TARGET 		= 	bat_server

DEFINES += A5K_SYS
unix{
	CURRENT_PROJECT_PATH = $$system(pwd)
}
win32{
	CURRENT_PROJECT_PATH = .
}
!include($$(SMARTSYS_HOME)/dev/mkspecs/qmake.conf){
         error(the file $$(SMARTSYS_HOME)/dev/mkspecs/qmake.conf is not exist!!)
}

include(../agvc.pri)
include (../tactics/$$logic/self.pri)

CONFIG    += thread
CONFIG    -= qt

DEPENDPATH += . include src

INCLUDEPATH +=	. ./include ./include/Eigen

# Input
HEADERS += include/bat_server.h \
           include/bat_calc.h \
           include/bat_rtdbcalc.h \
           include/bat_hiscalc.h \
           include/bat_rtdbdef.h \
           include/bat_config.h \
		   include/bat_math.h \
		   include/bat_taos.h \
		   include/stringutils.h \
		   include/bat_prewarning.h
           
SOURCES += src/bat_server.cpp \
           src/bat_rtdbcalc.cpp \
		   src/bat_rtdbvpp.cpp \
           src/bat_hiscalc.cpp \
           src/bat_math.cpp \
           src/bat_config.cpp \
           src/bat_taos.cpp \
		   src/stringutils.cpp \
		   src/bat_prewarning.cpp \
           src/main.cpp
           

LIBS += -leproc -ldatastream -lsysrun -lemsg -lrtdb -lrtdb_client -lscada_api -ldbclient -lsmsgpack -leco \
		-ldbsync -lrtdb_sync_by_app -lsamplequerycli -ltaos
