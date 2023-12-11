CFLAGS					=	-Wall -std=c++20 -g

TARGET_LIBSTATIC		=	lambda.a
TARGET_LIBSHARED		=	lambda.dll

LIB_OBJECTS				=	core/core.a extra/extra.a

LINK_COMPRESS_LIBS		=	-lz -lbrotlicommon -lbrotlidec -lbrotlienc
LINK_SYSTEM_LIBS		=	-lws2_32

.PHONY: all all-before all-after action-custom
all: all-before $(TARGET_LIBSTATIC) all-after

include Makefile.core.mk
include Makefile.extra.mk
include Makefile.test.mk

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

cleanw: action-custom
	del /S *.o *.exe *.a *.dll *.res

# static lib build
libstatic: $(TARGET_LIBSTATIC)

$(TARGET_LIBSTATIC): $(LIB_OBJECTS)
	ar rvs $(TARGET_LIBSTATIC) $(LIB_OBJECTS)

# shared lib build
libshared: $(TARGET_LIBSHARED)

$(TARGET_LIBSHARED): $(LIB_OBJECTS)
	g++ $(CFLAGS) $(LIB_OBJECTS) $(EXT_COMPRESS_LIBS) $(LINK_SYSTEM_LIBS) -s -shared -o $(TARGET_LIBSHARED) -Wl,--out-implib,lib$(TARGET_LIBSHARED).a
