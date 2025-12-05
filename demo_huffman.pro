QT += core gui widgets
CONFIG += c++11

SOURCES += huffman_main.cpp \
           huffmantree.cpp \
           huffmanwidget.cpp

HEADERS += huffmantree.h \
           huffmanwidget.h

TARGET = HuffmanDemo
TEMPLATE = app
