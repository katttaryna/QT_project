TEMPLATE = app
TARGET = player

QT       += core gui \
            multimedia\
            multimediawidgets\
            widgets\
            network\
            xml



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


SOURCES += \
    main.cpp \
    player.cpp \
    playercontrols.cpp \
    playlistmodel.cpp

HEADERS += \
    player.h \
    playercontrols.h \
    playlistmodel.h

FORMS += \
    player.ui

//macx: ICON = <b>.icns


qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



#TEMPLATE = app
#TARGET = player

#QT += network \
#      xml \
#      multimedia \
#      multimediawidgets \
#      widgets

#HEADERS = \
#    player.h \
#    playercontrols.h \
#    playlistmodel.h \
#    videowidget.h \
#    histogramwidget.h
#SOURCES = main.cpp \
#    player.cpp \
#    playercontrols.cpp \
#    playlistmodel.cpp \
#    videowidget.cpp \
#    histogramwidget.cpp

#target.path = $$[QT_INSTALL_EXAMPLES]/multimediawidgets/player
#INSTALLS += target

