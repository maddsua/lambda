
LIBNAME					=	lambda
TEMPBIN					=	./.bin/
CFLAGS					=	-Wall -Werror -std=c++20
LIB_DEPS				=	$(LIB_CORE_DEPS) $(LIB_EXTRA_DEPS)
EXTERNAL_LIBS			=	-lz -lbrotlicommon -lbrotlidec -lbrotlienc
LAMBDA_LIBSTATIC		=	$(LIBNAME).a
LAMBDA_LIBSHARED		=	$(LIBNAME)$(DLLEXT)
BUILD_TARGET			=	$(target)

ifeq ($(BUILD_TARGET),prod)
	CFLAGS				+=	-s
else
	CFLAGS				+=	-g
endif

ifeq ($(OS),Windows_NT)
	CLEAN_COMMAND		=	del /S *.o *.exe *.a *.dll *.so *.res
	EXEEXT				=	.exe
	DLLEXT				=	.dll
	LINK_SYSTEM_LIBS	=	-lws2_32
	WINDOWS_DLL_DEPS	=	dllinfo.res
	DLL_LDFLAGS			=	-Wl,--out-implib,lib$(LAMBDA_LIBSHARED).a
	BINRES_TARGET		=	pe-x86-64
else
	CLEAN_COMMAND		=	rm -rf *.o *.exe *.a *.dll *.so *.res
	BINRES_TARGET		=	elf64-x86-64 
	DLLEXT				=	.so
	CFLAGS				+=	-fPIC
endif

.PHONY: all all-before all-after action-custom
all: all-before $(LAMBDA_LIBSTATIC) all-after

clean: action-custom
	$(CLEAN_COMMAND)

include Makefile.core.mk
include Makefile.extra.mk
include Makefile.test.mk
include Makefile.tools.mk
include Makefile.examples.mk

# shared lib build
libshared: $(LAMBDA_LIBSHARED)

$(LAMBDA_LIBSHARED): $(LIB_DEPS) $(WINDOWS_DLL_DEPS)
	g++ $(CFLAGS) $(LIB_DEPS) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) $(WINDOWS_DLL_DEPS) -shared -o $(LAMBDA_LIBSHARED) $(DLL_LDFLAGS)

dllinfo.res: dllinfo.rc
	windres -i dllinfo.rc --input-format=rc -o dllinfo.res -O coff

dllinfo.rc: $(TOOL_UPDATEDLLINFO_TARGET)
	$(TOOL_UPDATEDLLINFO_TARGET) --template=dllinfo.template.rc --info=version.hpp --output=dllinfo.rc

# static lib build
libstatic: $(LAMBDA_LIBSTATIC)

$(LAMBDA_LIBSTATIC): $(LIB_DEPS)
	ar rvs $(LAMBDA_LIBSTATIC) $(LIB_DEPS)
