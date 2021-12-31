#-------------------------------------------------
#
# Project created by QtCreator 2016-05-27T04:24:14
#
#-------------------------------------------------

# Find Boost library.

# Try to use qmake variable's value.
_BOOST_ROOT = $$BOOST_ROOT
isEmpty(_BOOST_ROOT) {
    message(\"Boost Library\" qmake value not detected...)

    # Try to use the system environment value.
    _BOOST_ROOT = $$(BOOST_ROOT)
}

isEmpty(_BOOST_ROOT) {
    message(\"Boost Library\" environment variable not detected...)
} else {
    message(\"Boost Library\" detected in BOOST_ROOT = \"$$_BOOST_ROOT\")
    INCLUDEPATH += $$_BOOST_ROOT
}

QT += core gui network widgets

TARGET = Borderless
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++17
INCLUDEPATH += $$PWD/src

SOURCES +=  src/DirectoryListing.cpp          \
            src/ImageViewerApplication.cpp    \
            src/ImageViewport.cpp             \
            src/LoadedImage.cpp               \
            src/Settings.cpp                  \
            src/ShortcutsSettings.cpp         \
            src/main.cpp                      \
            src/MainWindow.cpp                \
            src/MainWindowMovement.cpp        \
            src/MainWindowSettings.cpp        \
            src/MainWindowShortcuts.cpp       \
            src/OptionsDialog.cpp             \
            src/RotateDialog.cpp              \
            src/Shortcuts.cpp                 \
            src/Streams.cpp                   \
            src/SingleInstanceApplication.cpp \
            src/ZoomModeDropDown.cpp          \
            src/ProtocolModule.cpp

HEADERS += src/DirectoryListing.h          \
           src/Enums.h                     \
           src/GenericException.h          \
           src/ImageViewerApplication.h    \
           src/ImageViewport.h             \
           src/LoadedImage.h               \
           src/MainWindow.h                \
           src/Misc.h                      \
           src/OptionsDialog.h             \
           src/Quadrangular.h              \
           src/Settings.h                  \
           src/resource.h                  \
           src/RotateDialog.h              \
           src/ShortcutInfo.h              \
           src/Shortcuts.h                 \
           src/SingleInstanceApplication.h \
           src/stdafx.h                    \
           src/StreamRedirector.h          \
           src/Streams.h                   \
           src/ZoomModeDropDown.h          \
           src/ProtocolModule.h

FORMS += src/InfoDialog.ui        \
         src/MainWindow.ui        \
         src/OptionsDialog.ui     \
         src/RotateDialog.ui

RESOURCES += src/resources.qrc
