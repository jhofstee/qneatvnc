QT += quickwidgets

SOURCES += \
    main.cpp

resources.files = main.qml
resources.prefix = /$${TARGET}
RESOURCES += resources

include(../../qneatvnc.pri)
