TEMPLATE = app
TARGET = bruntagger
QT += core \
    gui
HEADERS += itemdelegates.h \
    picsmodel.h \
    picsview.h \
    tagsview.h \
    version.h \
    tagsmodel.h \
    mp3.h \
    bruntagger.h
SOURCES += itemdelegates.cpp \
    picsmodel.cpp \
    picsview.cpp \
    tagsview.cpp \
    tagsmodel.cpp \
    mp3.cpp \
    main.cpp \
    bruntagger.cpp
FORMS += bruntagger.ui
RESOURCES += bruntagger.qrc
LIBS += -ltag \
    -lkdecore \
    -lboost_regex
TRANSLATIONS = en_US.ts
CODECFORTR = ISO-8859-5
unix { 
    isEmpty(PREFIX):PREFIX = /usr/local
    binaries.path = $$PREFIX/bin
    binaries.files = $$TARGET
    INSTALLS += binaries
}
