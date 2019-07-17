TEMPLATE = app
#CONFIG += console
win32:QMAKE_LFLAGS_CONSOLE = -Wl,-subsystem,windows
win32:QMAKE_LFLAGS_WINDOWS = -Wl,-subsystem,windows

CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += precompile_header

DEFINES += UNICODE _UNICODE

APP_NAME = slot_machine

CONFIG(debug, debug|release) {
	CONF_NAME = debug
    TARGET_POSTFIX = d
	DEFINES += _DEBUG
}
else {
	CONF_NAME = release
	DEFINES += NDEBUG
}

QMAKE_SPEC_T = $$[QMAKE_SPEC]

message($$QMAKE_SPEC_T)
message($$QT_ARCH)

# Target name
TARGET = $$APP_NAME"_"$$TARGET_POSTFIX

# Destination directories
DESTDIR = $$OUT_PWD/$$CONF_NAME
DLLDESTDIR = bin

# Moc output directory
MOC_DIR = tmp/$$CONF_NAME

# Resource compiler directory
RCC_DIR = tmp

# Temp objects output directory
OBJECTS_DIR = obj/$$CONF_NAME

# Istalls
release:extra_bin.path = $$OUT_PWD/bin
release:extra_bin.files = $$DESTDIR/$$APP_PRJ_NAME
release:INSTALLS += extra_bin

# Precompiled header
PRECOMPILED_HEADER = forwardrefs.h

precompile_header:!isEmpty(PRECOMPILED_HEADER) {
	DEFINES += USING_PCH
}

win32 {
INCLUDEPATH += D:/projects/3rd_party/freeglut/include \
    D:/projects/3rd_party/SOIL/src \
    # nvidia GL extension
    'C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v8.0/extras/CUPTI/include'
}

SOURCES += main.cpp \
    modelview.cpp \
    system.cpp

HEADERS += \
    modelview.h \
    singleton.h \
    system.h \
    forwardrefs.h \
    bmpimage.h \
    tgaimage.h \
    baseimage.h

DISTFILES += \
    slot_machine.pro.user

win32 {
LIBS += -LD:/projects/3rd_party/freeglut/bin \
    -lfreeglut_static$$TARGET_POSTFIX \
    -lopengl32 \
    -LD:/projects/3rd_party/SOIL/lib \
    -lsoil$$TARGET_POSTFIX
}

unix {
LIBS += \
	-lGL	\
	-lGLU	\
	-lglut
}
