
LIB_CORE_ERROR			=	core/error.a
LIB_CORE_ERROR_DEPS		=	core/error/apierror.o

# error handling
$(LIB_CORE_ERROR): $(LIB_CORE_ERROR_DEPS)
	ar rvs $(LIB_CORE_ERROR) $(LIB_CORE_ERROR_DEPS)

core/error/apierror.o: core/error/apierror.cpp
	g++ -c $(CFLAGS) core/error/apierror.cpp -o core/error/apierror.o
