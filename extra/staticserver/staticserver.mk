
LIB_EXTRA_STATICSERVER			=	extra/staticserver.a
LIB_EXTRA_STATICSERVER_DEPS		=	extra/staticserver/staticserver.o

# storage stuff
$(LIB_EXTRA_STATICSERVER): $(LIB_EXTRA_STATICSERVER_DEPS)
	ar rvs $(LIB_EXTRA_STATICSERVER) $(LIB_EXTRA_STATICSERVER_DEPS)

extra/staticserver/staticserver.o: extra/staticserver/staticserver.cpp
	g++ -c $(CFLAGS) extra/staticserver/staticserver.cpp -o extra/staticserver/staticserver.o
