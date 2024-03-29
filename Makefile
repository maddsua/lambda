# lambda main makefile

LIBNAME						=	lambda
CFLAGS						=	-Wall -Werror -std=c++20 $(if $(filter $(target),prod),-s,-g)
LIB_DEPS					=	$(LIB_CORE_DEPS) $(LIB_EXTRA_DEPS)
LAMBDA_LIBSTATIC			=	$(LIBNAME).a
LAMBDA_LIBSHARED			=	$(LIBNAME)$(DLLEXT)
BUILD_TARGET				=	$(target)
BUILD_ENABLE_COMPRESSION	=	$(if $(filter $(compression),disabled),false,true)

ifeq ($(BUILD_ENABLE_COMPRESSION),true)
	EXTERNAL_LIBS		+=	-lz -lbrotlicommon -lbrotlidec -lbrotlienc
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
	EXEPFX				=	./
	CLEAN_COMMAND		=	rm -rf *.o *.exe *.a *.dll *.so *.res
	BINRES_TARGET		=	elf64-x86-64 
	DLLEXT				=	.so
	CFLAGS				+=	-fPIC
endif

.PHONY: all all-before all-after action-custom
all: all-before libshared all-after

clean: action-custom
	$(CLEAN_COMMAND)

include core/core.mk
include extra/extra.mk
include examples/examples.mk
include tools/tools.mk
include tests/tests.mk

# shared lib build
libshared: $(LAMBDA_LIBSHARED)

$(LAMBDA_LIBSHARED): $(LIB_DEPS) $(WINDOWS_DLL_DEPS)
	g++ $(CFLAGS) $(DLL_LDFLAGS) $(LIB_DEPS) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) $(WINDOWS_DLL_DEPS) -shared -o $(LAMBDA_LIBSHARED)

dllinfo.res: dllinfo.rc
	windres -i dllinfo.rc --input-format=rc -o dllinfo.res -O coff

dllinfo.rc: $(TOOL_UPDATEDLLINFO_TARGET)
	$(TOOL_UPDATEDLLINFO_TARGET) --info=version.hpp --output=dllinfo.rc --template=tools/updatedllinfo/dllinfo.template

# static lib build
libstatic: $(LAMBDA_LIBSTATIC)

$(LAMBDA_LIBSTATIC): $(LIB_DEPS)
	ar rvs $(LAMBDA_LIBSTATIC) $(LIB_DEPS)

# generate single file include
gensfi: $(TOOL_GENSFI_TARGET)
	$(TOOL_GENSFI_TARGET) --entrypoint=lambda.hpp --output=lambda_sfi.hpp --template=tools/gensfi/headerfile.template
