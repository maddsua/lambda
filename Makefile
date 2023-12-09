CFLAGS			=	-Wall -std=c++20 -g
LIB_TARGET		=	octo.a
LIB_OBJECTS		=	core/core.a extra/extra.a

.PHONY: all all-before all-after action-custom
all: all-before $(LIB_TARGET) all-after

include Makefile.core.mk
include Makefile.extra.mk
include Makefile.test.mk

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

cleanw: action-custom
	del /S *.o *.exe *.a *.dll *.res

$(LIB_TARGET): $(LIB_OBJECTS)
	ar rvs $(LIB_TARGET) $(LIB_OBJECTS)
