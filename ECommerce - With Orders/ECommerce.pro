# Specifies the Qt modules required by the application.
QT       += core gui widgets

# Sets the target executable name.
TARGET = ECommerceApp
# Specifies that the output is an application.
TEMPLATE = app

# Lists the source code files (.cpp) to be compiled.
# Ensure ALL your .cpp files are listed here.
SOURCES += main.cpp \
           mainwindow.cpp \
           logindialog.cpp \
           checkoutdialog.cpp \
           orderhistorydialog.cpp # <<< MAKE SURE THIS IS PRESENT

# Lists the header files (.h) used in the project.
# Ensure ALL your .h files that contain Q_OBJECT are listed here.
HEADERS  += mainwindow.h \
            logindialog.h \
            checkoutdialog.h \
            orderhistorydialog.h # <<< MAKE SURE THIS IS PRESENT

# Enables C++11 features and debug configuration.
CONFIG += c++11 debug

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
