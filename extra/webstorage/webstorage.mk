
LIB_EXTRA_STORAGE		=	extra/storage.a
LIB_EXTRA_STORAGE_DEPS	=	extra/webstorage/driver.o extra/webstorage/interface.o

# storage stuff
$(LIB_EXTRA_STORAGE): $(LIB_EXTRA_STORAGE_DEPS)
	ar rvs $(LIB_EXTRA_STORAGE) $(LIB_EXTRA_STORAGE_DEPS)

extra/webstorage/driver.o: extra/webstorage/driver.cpp
	g++ -c $(CFLAGS) extra/webstorage/driver.cpp -o extra/webstorage/driver.o

extra/webstorage/interface.o: extra/webstorage/interface.cpp
	g++ -c $(CFLAGS) extra/webstorage/interface.cpp -o extra/webstorage/interface.o
