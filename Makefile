
CFLAGS					=	-Wall -std=c++20 -g

LAMBDA_LIBSTATIC		=	lambda.a
LAMBDA_LIBSHARED		=	lambda.dll

LAMBDA_DEPS				=	$(LIB_CORE_DEPS) $(LIB_EXTRA_DEPS)

LINK_COMPRESS_LIBS		=	-lz -lbrotlicommon -lbrotlidec -lbrotlienc
LINK_SYSTEM_LIBS		=	-lws2_32

.PHONY: all all-before all-after action-custom
all: all-before $(LAMBDA_LIBSTATIC) all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

cleanw: action-custom
	del /S *.o *.exe *.a *.dll *.res

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
	g++ $(CFLAGS) $(LAMBDA_DEPS) $(LINK_COMPRESS_LIBS) $(LINK_SYSTEM_LIBS) dllinfo.res -s -shared -o $(LAMBDA_LIBSHARED) -Wl,--out-implib,lib$(LAMBDA_LIBSHARED).a

dllinfo.res: dllinfo.rc
	windres -i dllinfo.rc --input-format=rc -o dllinfo.res -O coff

dllinfo.rc: updatedllinfo.exe
	updatedllinfo.exe --template=dllinfo.template.rc --info=lambda_version.hpp --output=dllinfo.rc
