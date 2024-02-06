
LIB_EXTRA_KVSTORAGE			=	extra/storage.a
LIB_EXTRA_KVSTORAGE_DEPS	=	extra/kvstorage/driver.o extra/kvstorage/interface.o

# storage stuff
$(LIB_EXTRA_KVSTORAGE): $(LIB_EXTRA_KVSTORAGE_DEPS)
	ar rvs $(LIB_EXTRA_KVSTORAGE) $(LIB_EXTRA_KVSTORAGE_DEPS)

extra/kvstorage/driver.o: extra/kvstorage/driver.cpp
	g++ -c $(CFLAGS) extra/kvstorage/driver.cpp -o extra/kvstorage/driver.o

extra/kvstorage/interface.o: extra/kvstorage/interface.cpp
	g++ -c $(CFLAGS) extra/kvstorage/interface.cpp -o extra/kvstorage/interface.o
