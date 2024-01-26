LIB_EXTRA				=	extra/extra.a
LIB_EXTRA_DEPS			=	$(LIB_EXTRA_STORAGE_DEPS)

LIB_EXTRA_STORAGE		=	extra/storage.a
LIB_EXTRA_STORAGE_DEPS	=	extra/storage/localStorage.o extra/storage/sessionStorage.o


# extra object
lambda.extra: $(LIB_EXTRA)

$(LIB_EXTRA): $(LIB_EXTRA_DEPS)
	ar rvs $(LIB_EXTRA) $(LIB_EXTRA_DEPS)


# storage stuff
$(LIB_EXTRA_STORAGE): $(LIB_EXTRA_STORAGE_DEPS)
	ar rvs $(LIB_EXTRA_STORAGE) $(LIB_EXTRA_STORAGE_DEPS)

extra/storage/localStorage.o: extra/storage/localStorage.cpp
	g++ -c $(CFLAGS) extra/storage/localStorage.cpp -o extra/storage/localStorage.o

extra/storage/sessionStorage.o: extra/storage/sessionStorage.cpp
	g++ -c $(CFLAGS) extra/storage/sessionStorage.cpp -o extra/storage/sessionStorage.o
