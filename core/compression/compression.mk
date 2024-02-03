
LIB_CORE_COMPRESS		=	core/compression.a
LIB_CORE_COMPRESS_DEPS	=	core/compression/streams.o core/compression/brotli.o core/compression/zlib.o

# compression stuff
$(LIB_CORE_COMPRESS): $(LIB_CORE_COMPRESS_DEPS)
	ar rvs $(LIB_CORE_COMPRESS) $(LIB_CORE_COMPRESS_DEPS)

core/compression/streams.o: core/compression/streams.cpp
	g++ -c $(CFLAGS) core/compression/streams.cpp -o core/compression/streams.o

core/compression/brotli.o: core/compression/brotli.cpp
	g++ -c $(CFLAGS) core/compression/brotli.cpp -o core/compression/brotli.o

core/compression/zlib.o: core/compression/zlib.cpp
	g++ -c $(CFLAGS) core/compression/zlib.cpp -o core/compression/zlib.o
