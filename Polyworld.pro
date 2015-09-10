include(Polyworld.pri)

for(SOURCE_DIR, SOURCE_DIRS) {
	HEADERS += $${SOURCE_DIR}/*.h
	SOURCES += $${SOURCE_DIR}/*.cp
}

TEMPLATE = lib
CONFIG += shared
QT += opengl
QMAKE_EXT_CPP = .cp
QMAKE_CXXFLAGS += -std=gnu++0x -fopenmp
QMAKE_LFLAGS += -Wl,-no-undefined,--enable-runtime-pseudo-reloc -fopenmp
Debug:DEFINES += POLYWORLD_DEBUG
Release:DEFINES += POLYWORLD_RELEASE
INCLUDEPATH += /mingw/include/GL /qt/include/QtOpenGL /python/include $$SOURCE_DIRS
LIBS += -ldl -lgsl -lpsapi -lpthread -L/python/libs -lpython27 -lwsock32 -lz

debug_main.target = debug-main
debug_main.depends = FORCE
debug_main.commands = $(MAKE) -f $(MAKEFILE).Debug -f Makefile.main main
QMAKE_EXTRA_TARGETS += debug_main

release_main.target = release-main
release_main.depends = FORCE
release_main.commands = $(MAKE) -f $(MAKEFILE).Release -f Makefile.main main
QMAKE_EXTRA_TARGETS += release_main
