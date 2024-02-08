
LIB_CORE_COMPRESS		=	core/compression.a
LIB_CORE_COMPRESS_DEPS	=	core/compression/brotli.o core/compression/zlib_oneshot.o core/compression/zlib_stream_compress.o core/compression/zlib_stream_decompress.o

# compression stuff
$(LIB_CORE_COMPRESS): $(LIB_CORE_COMPRESS_DEPS)
	ar rvs $(LIB_CORE_COMPRESS) $(LIB_CORE_COMPRESS_DEPS)

core/compression/brotli.o: core/compression/brotli.cpp
	g++ -c $(CFLAGS) core/compression/brotli.cpp -o core/compression/brotli.o

core/compression/zlib_oneshot.o: core/compression/zlib_oneshot.cpp
	g++ -c $(CFLAGS) core/compression/zlib_oneshot.cpp -o core/compression/zlib_oneshot.o

core/compression/zlib_stream_compress.o: core/compression/zlib_stream_compress.cpp
	g++ -c $(CFLAGS) core/compression/zlib_stream_compress.cpp -o core/compression/zlib_stream_compress.o

core/compression/zlib_stream_decompress.o: core/compression/zlib_stream_decompress.cpp
	g++ -c $(CFLAGS) core/compression/zlib_stream_decompress.cpp -o core/compression/zlib_stream_decompress.o
