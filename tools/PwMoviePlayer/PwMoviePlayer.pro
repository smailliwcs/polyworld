include(../../Polyworld.pri)

HEADERS = \
	GLWidget.h \
	MainWindow.h \
	PwMoviePlayer.h
SOURCES = \
	GLWidget.cp \
	MainWindow.cp \
	PwMoviePlayer.cp

QT += opengl
QMAKE_EXT_CPP = .cp
QMAKE_PRE_LINK = $(COPY_FILE) ../../$(OBJECTS_DIR)Polyworld.dll $(OBJECTS_DIR)
QMAKE_CLEAN += $(OBJECTS_DIR)Polyworld.dll
INCLUDEPATH += /mingw/include/GL
for(SOURCE_DIR, SOURCE_DIRS) {
	INCLUDEPATH += ../../$${SOURCE_DIR}
}
LIBS += -L$(OBJECTS_DIR) -lPolyworld
