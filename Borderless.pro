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
QMAKE_EXTRA_TARGETS += SerializationCode
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11
INCLUDEPATH += $$PWD/serialization/postsrc $$PWD/src

SOURCES +=  src/ClangErrorMessage.cpp               \
            src/DirectoryListing.cpp                \
            src/ImageViewerApplication.cpp          \
            src/ImageViewport.cpp                   \
            src/LoadedImage.cpp                     \
            src/main.cpp                            \
            src/MainWindow.cpp                      \
            src/MainWindowMovement.cpp              \
            src/MainWindowSettings.cpp              \
            src/MainWindowShortcuts.cpp             \
            src/OptionsDialog.cpp                   \
            src/RotateDialog.cpp                    \
            src/Shortcuts.cpp                       \
            src/Streams.cpp                         \
            src/SingleInstanceApplication.cpp       \
            src/ZoomModeDropDown.cpp                \
            src/plugin-core/capi.cpp                \
            src/plugin-core/ImageStore.cpp          \
            src/plugin-core/PluginCoreState.cpp     \
            src/serialization/Implementations.cpp   \
            src/serialization/Inlining.cpp          \
            src/serialization/MainSettings.cpp      \
            src/serialization/ShortcutsSettings.cpp \
            src/serialization/WindowState.cpp

HEADERS += src/ClangErrorMessage.hpp         \
           src/DirectoryListing.h            \
           src/Enums.h                       \
           src/GenericException.h            \
           src/ImageViewerApplication.h      \
           src/ImageViewport.h               \
           src/LoadedImage.h                 \
           src/MainWindow.h                  \
           src/Misc.h                        \
           src/OptionsDialog.h               \
           src/Quadrangular.h                \
           src/resource.h                    \
           src/RotateDialog.h                \
           src/ShortcutInfo.h                \
           src/Shortcuts.h                   \
           src/SingleInstanceApplication.h   \
           src/stdafx.h                      \
           src/StreamRedirector.h            \
           src/Streams.h                     \
           src/ZoomModeDropDown.h            \
           src/plugin-core/capi.h            \
           src/plugin-core/ImageStore.h      \
           src/plugin-core/PluginCoreState.h \
           src/plugin-core/Cpp/main.h        \
           src/plugin-core/Lua/main.h

FORMS += src/ClangErrorMessage.ui \
         src/InfoDialog.ui        \
         src/MainWindow.ui        \
         src/OptionsDialog.ui     \
         src/RotateDialog.ui

RESOURCES += src/resources.qrc

LASC.target   = $$PWD/LAS
#LASC.depends  = FORCE
LASC.commands = chmod +x build_las.sh; ./build_las.sh

SerializationCode.target   = $$PWD/src/serialization/settings.generated.cpp
SerializationCode.depends  = LASC $$PWD/src/serialization/settings.txt
SerializationCode.commands = chmod +x build_serialization.sh; ./build_serialization.sh
