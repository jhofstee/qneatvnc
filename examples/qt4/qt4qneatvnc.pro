QT += declarative

SOURCES += \
    main.cpp

OTHER_FILES += \
    main.qml

DEFINES += "PWD=$${PWD}"

include(../../qneatvnc.pri)


*g++* {
    # suppress the mangling of va_arg has changed for gcc 4.4
    QMAKE_CXXFLAGS += -Wno-psabi -std=c++11

    # gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
    QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

    QMAKE_CXX += -Wno-class-memaccess -Wno-deprecated-copy
}
