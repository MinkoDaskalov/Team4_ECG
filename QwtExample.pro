# Qt project file - qmake uses his to generate a Makefile

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QwtExample

LIBS += -lqwt -lm

HEADERS += window.h \
    bcm2835.h \
    gpio-sysfs.h \
    gz_clk.h \
    adcreader.h \
    Iir.h \
    iir/Bessel.h \
    iir/Biquad.h \
    iir/Butterworth.h \
    iir/Cascade.h \
    iir/ChebyshevI.h \
    iir/ChebyshevII.h \
    iir/Common.h \
    iir/config.h \
    iir/Custom.h \
    iir/Elliptic.h \
    iir/Layout.h \
    iir/Legendre.h \
    iir/MathSupplement.h \
    iir/PoleFilter.h \
    iir/RBJ.h \
    iir/RootFinder.h \
    iir/State.h \
    iir/Types.h \
    iir/Utilities.h

SOURCES += main.cpp window.cpp \
    gpio-sysfs.cpp \
    gz_clk.cpp \
    bcm2835.c \
    adcreader.cpp \
    iir/Bessel.cpp \
    iir/Biquad.cpp \
    iir/Butterworth.cpp \
    iir/Cascade.cpp \
    iir/ChebyshevI.cpp \
    iir/ChebyshevII.cpp \
    iir/Custom.cpp \
    iir/Elliptic.cpp \
    iir/Legendre.cpp \
    iir/PoleFilter.cpp \
    iir/RBJ.cpp \
    iir/RootFinder.cpp \
    iir/State.cpp

CONFIG += qwt

INCLUDEPATH +="/usr/local/qwt-6.1.2/include"

LIBS+= -L/usr/local/qwt-6.1.2/lib -lqwt

