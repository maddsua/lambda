
TEMPBIN = .bin/
CFLAGS					=	-Wall -Werror -std=c++20 -g
LAMBDA_DEPS				=	$(LIB_CORE_DEPS) $(LIB_EXTRA_DEPS)
LINK_OTHER_LIBS			=	-lz -lbrotlicommon -lbrotlidec -lbrotlienc

ifeq ($(OS),Windows_NT)
	EXEEXT = .exe
	DLLEXT = .dll
	LINK_SYSTEM_LIBS	=	-lws2_32
else
	DLLEXT = .so
endif

LAMBDA_LIBSTATIC		=	lambda.a
LAMBDA_LIBSHARED		=	lambda$(DLLEXT)

.PHONY: all all-before all-after action-custom
all: all-before $(LAMBDA_LIBSTATIC) all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.so *.res

cleanw: action-custom
	del /S *.o *.exe *.a *.dll *.so *.res

include Makefile.core.mk
include Makefile.extra.mk
include Makefile.test.mk
include Makefile.tools.mk
include Makefile.examples.mk

# static lib build
libstatic: $(LAMBDA_LIBSTATIC)

$(LAMBDA_LIBSTATIC): $(LAMBDA_DEPS)
	ar rvs $(LAMBDA_LIBSTATIC) $(LAMBDA_DEPS)

# shared lib build
libshared: $(LAMBDA_LIBSHARED)

$(LAMBDA_LIBSHARED): $(LAMBDA_DEPS) dllinfo.res
	g++ $(CFLAGS) $(LAMBDA_DEPS) $(LINK_OTHER_LIBS) $(LINK_SYSTEM_LIBS) dllinfo.res -s -shared -o $(LAMBDA_LIBSHARED) -Wl,--out-implib,lib$(LAMBDA_LIBSHARED).a

dllinfo.res: dllinfo.rc
	windres -i dllinfo.rc --input-format=rc -o dllinfo.res -O coff

dllinfo.rc: updatedllinfo$(EXEEXT)
	updatedllinfo$(EXEEXT) --template=dllinfo.template.rc --info=lambda_version.hpp --output=dllinfo.rc
