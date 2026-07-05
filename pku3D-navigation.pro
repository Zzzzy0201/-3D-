QT += widgets
QT += opengl
QT += openglwidgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cameramanager.cpp \
    campus3dwidget.cpp \
    detourdialog.cpp \
    gldrawingengine.cpp \
    main.cpp \
    mainwindow.cpp \
    maploader.cpp \
    maptransformer.cpp \
    mapwidget.cpp \
    preference_order_dialog.cpp \
    routedialog.cpp \
    routeplanner.cpp \
    small_mapwidget.cpp \
    waypointreorderwidget.cpp

HEADERS += \
    building.h \
    cameramanager.h \
    campus3dwidget.h \
    detourdialog.h \
    gldrawingengine.h \
    mainwindow.h \
    maploader.h \
    maptransformer.h \
    mapwidget.h \
    preference_order_dialog.h \
    routedialog.h \
    routeplanner.h \
    small_mapwidget.h \
    waypointreorderwidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

LIBS += -lopengl32 -lglu32 -lgdi32
