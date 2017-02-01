TARGET = preyeditor
TEMPLATE = app

QT += core gui widgets
QT += qml quick

CONFIG += c++11

HEADERS += \
    jsondelegate.h \
    varianttreeitem.h \
    varianttreemodel.h \
    varianttreewidget.h \
    yamldelegate.h

SOURCES += \
    jsondelegate.cpp \
    varianttreeitem.cpp \
    varianttreemodel.cpp \
    varianttreewidget.cpp \
    yamldelegate.cpp

SOURCES += main.cpp

RESOURCES += resources.qrc

# Hardened options
# QMAKE_LFLAGS += -nopie
# QMAKE_POST_LINK += /usr/sbin/paxctl-ng -mps preyeditor

# TODO: add yaml functions
# LIBS += -lyaml-cpp
