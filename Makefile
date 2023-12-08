CFLAGS			=	-std=c++20
LIB_TARGET		=	octo.a
LIB_OBJECTS		=	core.o

.PHONY: all all-before all-after action-custom
all: all-before $(LIB_TARGET) all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

cleanw: action-custom
	del /S *.o *.exe *.a *.dll *.res

$(LIB_TARGET): $(LIB_OBJECTS)
	ar rvs $(LIB_TARGET) $(LIB_OBJECTS)

include Makefile.core.mk
