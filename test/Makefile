MKPATH=$(abspath $(lastword $(MAKEFILE_LIST)))

SOURCEDIR=src
TOPDIR=../../..
INCDIR=-I$(TOPDIR)/include
LIBDIR=-L$(TOPDIR)/lib

LIBS_LINUX=-lfiAsset
LIBS_W32=-lfiAsset
LIBS_OSX=-lfiAsset

### Input Source Files
C_FILES=
CXX_FILES=\
	main.cpp
OBJC_FILES=
OBJCXX_FILES=
#C_FILES_LINUX
#CXX_FILES_LINUX
#C_FILES_WIN
#CXX_FILES_WIN
#C_FILES_OSX
#CXX_FILES_OSX
#OBJC_FILES_OSX
#OBJCXX_FILES_OSX

### Targets / Output / Resource folders and files
TARGET=fipack
CLEAN_TARGET=$(TARGET)
RESOURCES=

### Specify project dependencies here
DEP_WIN32=
DEP_OSX=
DEP_LINUX=
DEPENDENCIES=\
	$(TOPDIR)/projects/fiAsset

### Post Build Steps
POSTBUILD=
POSTBUILDCLEAN=

include $(TOPDIR)/templates/Makefile_include_application
