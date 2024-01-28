LIB_EXTRA				=	extra/extra.a
LIB_EXTRA_DEPS			=	$(LIB_EXTRA_STORAGE_DEPS)

LIB_EXTRA_STORAGE		=	extra/storage.a
LIB_EXTRA_STORAGE_DEPS	=	extra/webstorage/localStorage.o extra/webstorage/sessionStorage.o


# extra object
lambda.extra: $(LIB_EXTRA)

$(LIB_EXTRA): $(LIB_EXTRA_DEPS)
	ar rvs $(LIB_EXTRA) $(LIB_EXTRA_DEPS)


# storage stuff
$(LIB_EXTRA_STORAGE): $(LIB_EXTRA_STORAGE_DEPS)
	ar rvs $(LIB_EXTRA_STORAGE) $(LIB_EXTRA_STORAGE_DEPS)

extra/webstorage/localStorage.o: extra/webstorage/localStorage.cpp
	g++ -c $(CFLAGS) extra/webstorage/localStorage.cpp -o extra/webstorage/localStorage.o

extra/webstorage/sessionStorage.o: extra/webstorage/sessionStorage.cpp
	g++ -c $(CFLAGS) extra/webstorage/sessionStorage.cpp -o extra/webstorage/sessionStorage.o
