include Makefile.core.mk
include Makefile.extra.mk
include Makefile.test.mk

CFLAGS					=	-Wall -std=c++20 -g

LINK_COMPRESS_LIBS		=	-lz -lbrotlicommon -lbrotlidec -lbrotlienc
LINK_SYSTEM_LIBS		=	-lws2_32

LAMBDA_LIBSTATIC		=	lambda.a
LAMBDA_LIBSHARED		=	lambda.dll

LAMBDA_DEPS				=	$(CORE_DEPS) $(EXTRA_DEPS)

.PHONY: all all-before all-after action-custom
all: all-before $(LAMBDA_LIBSTATIC) all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

cleanw: action-custom
	del /S *.o *.exe *.a *.dll *.res

# static lib build
libstatic: $(LAMBDA_LIBSTATIC)

$(LAMBDA_LIBSTATIC): $(LAMBDA_DEPS)
	ar rvs $(LAMBDA_LIBSTATIC) $(LAMBDA_DEPS)

# shared lib build
libshared: $(LAMBDA_LIBSHARED)

$(LAMBDA_LIBSHARED): $(LAMBDA_DEPS)
	g++ $(CFLAGS) $(LAMBDA_DEPS) $(LINK_COMPRESS_LIBS) $(LINK_SYSTEM_LIBS) -s -shared -o $(LAMBDA_LIBSHARED) -Wl,--out-implib,lib$(LAMBDA_LIBSHARED).a
