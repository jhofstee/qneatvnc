INCLUDEPATH += $$PWD/inc

HEADERS += \
    $$PWD/inc/qneatvnc/qaml.hpp \
    $$PWD/inc/qneatvnc/qneatvnc.hpp

SOURCES += \
    $$PWD/src/qaml.cpp \
    $$PWD/src/qneatvnc.cpp \
    $$PWD/src/keys.cpp \
    $$PWD/src/keys.hpp \

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG = aml neatvnc pixman-1
}
