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
INCLUDEPATH += /mingw/include/GL /qt/include/QtOpenGL /python/include $$SOURCE_DIRS
LIBS += -ldl -lgsl -lpsapi -lpthread -L/python/libs -lpython27 -lwsock32 -lz
