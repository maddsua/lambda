#	This is an automatically generated file
#	Don't edit it directly - it may be overwritten
#	Modify template.mk to add something on top, or generate_deps_makefile.mjs to change automatic imports

CC = gcc
FLAGS = -std=c11

.PHONY: all all-before all-after action-custom

libs: deps/libbrotli-static.a deps/libz-static.a

clean: action-custom
	del /S *.o *.exe *.a *.dll *.res

deps/libbrotli-static.a: deps/brotli/c/common/constants.o deps/brotli/c/common/context.o deps/brotli/c/common/dictionary.o deps/brotli/c/common/platform.o deps/brotli/c/common/transform.o deps/brotli/c/dec/bit_reader.o deps/brotli/c/dec/decode.o deps/brotli/c/dec/huffman.o deps/brotli/c/dec/state.o deps/brotli/c/enc/backward_references.o deps/brotli/c/enc/backward_references_hq.o deps/brotli/c/enc/bit_cost.o deps/brotli/c/enc/block_splitter.o deps/brotli/c/enc/brotli_bit_stream.o deps/brotli/c/enc/cluster.o deps/brotli/c/enc/command.o deps/brotli/c/enc/compress_fragment.o deps/brotli/c/enc/compress_fragment_two_pass.o deps/brotli/c/enc/dictionary_hash.o deps/brotli/c/enc/encode.o deps/brotli/c/enc/encoder_dict.o deps/brotli/c/enc/entropy_encode.o deps/brotli/c/enc/fast_log.o deps/brotli/c/enc/histogram.o deps/brotli/c/enc/literal_cost.o deps/brotli/c/enc/memory.o deps/brotli/c/enc/metablock.o deps/brotli/c/enc/static_dict.o deps/brotli/c/enc/utf8_util.o
	ar rvs deps/libbrotli-static.a deps/temp/brotli/*.o

deps/brotli/c/common/constants.o: deps/brotli/c/common/constants.c
	$(CC) -c deps/brotli/c/common/constants.c -o deps/brotli/c/common/constants.o $(FLAGS)

deps/brotli/c/common/context.o: deps/brotli/c/common/context.c
	$(CC) -c deps/brotli/c/common/context.c -o deps/brotli/c/common/context.o $(FLAGS)

deps/brotli/c/common/dictionary.o: deps/brotli/c/common/dictionary.c
	$(CC) -c deps/brotli/c/common/dictionary.c -o deps/brotli/c/common/dictionary.o $(FLAGS)

deps/brotli/c/common/platform.o: deps/brotli/c/common/platform.c
	$(CC) -c deps/brotli/c/common/platform.c -o deps/brotli/c/common/platform.o $(FLAGS)

deps/brotli/c/common/transform.o: deps/brotli/c/common/transform.c
	$(CC) -c deps/brotli/c/common/transform.c -o deps/brotli/c/common/transform.o $(FLAGS)

deps/brotli/c/dec/bit_reader.o: deps/brotli/c/dec/bit_reader.c
	$(CC) -c deps/brotli/c/dec/bit_reader.c -o deps/brotli/c/dec/bit_reader.o $(FLAGS)

deps/brotli/c/dec/decode.o: deps/brotli/c/dec/decode.c
	$(CC) -c deps/brotli/c/dec/decode.c -o deps/brotli/c/dec/decode.o $(FLAGS)

deps/brotli/c/dec/huffman.o: deps/brotli/c/dec/huffman.c
	$(CC) -c deps/brotli/c/dec/huffman.c -o deps/brotli/c/dec/huffman.o $(FLAGS)

deps/brotli/c/dec/state.o: deps/brotli/c/dec/state.c
	$(CC) -c deps/brotli/c/dec/state.c -o deps/brotli/c/dec/state.o $(FLAGS)

deps/brotli/c/enc/backward_references.o: deps/brotli/c/enc/backward_references.c
	$(CC) -c deps/brotli/c/enc/backward_references.c -o deps/brotli/c/enc/backward_references.o $(FLAGS)

deps/brotli/c/enc/backward_references_hq.o: deps/brotli/c/enc/backward_references_hq.c
	$(CC) -c deps/brotli/c/enc/backward_references_hq.c -o deps/brotli/c/enc/backward_references_hq.o $(FLAGS)

deps/brotli/c/enc/bit_cost.o: deps/brotli/c/enc/bit_cost.c
	$(CC) -c deps/brotli/c/enc/bit_cost.c -o deps/brotli/c/enc/bit_cost.o $(FLAGS)

deps/brotli/c/enc/block_splitter.o: deps/brotli/c/enc/block_splitter.c
	$(CC) -c deps/brotli/c/enc/block_splitter.c -o deps/brotli/c/enc/block_splitter.o $(FLAGS)

deps/brotli/c/enc/brotli_bit_stream.o: deps/brotli/c/enc/brotli_bit_stream.c
	$(CC) -c deps/brotli/c/enc/brotli_bit_stream.c -o deps/brotli/c/enc/brotli_bit_stream.o $(FLAGS)

deps/brotli/c/enc/cluster.o: deps/brotli/c/enc/cluster.c
	$(CC) -c deps/brotli/c/enc/cluster.c -o deps/brotli/c/enc/cluster.o $(FLAGS)

deps/brotli/c/enc/command.o: deps/brotli/c/enc/command.c
	$(CC) -c deps/brotli/c/enc/command.c -o deps/brotli/c/enc/command.o $(FLAGS)

deps/brotli/c/enc/compress_fragment.o: deps/brotli/c/enc/compress_fragment.c
	$(CC) -c deps/brotli/c/enc/compress_fragment.c -o deps/brotli/c/enc/compress_fragment.o $(FLAGS)

deps/brotli/c/enc/compress_fragment_two_pass.o: deps/brotli/c/enc/compress_fragment_two_pass.c
	$(CC) -c deps/brotli/c/enc/compress_fragment_two_pass.c -o deps/brotli/c/enc/compress_fragment_two_pass.o $(FLAGS)

deps/brotli/c/enc/dictionary_hash.o: deps/brotli/c/enc/dictionary_hash.c
	$(CC) -c deps/brotli/c/enc/dictionary_hash.c -o deps/brotli/c/enc/dictionary_hash.o $(FLAGS)

deps/brotli/c/enc/encode.o: deps/brotli/c/enc/encode.c
	$(CC) -c deps/brotli/c/enc/encode.c -o deps/brotli/c/enc/encode.o $(FLAGS)

deps/brotli/c/enc/encoder_dict.o: deps/brotli/c/enc/encoder_dict.c
	$(CC) -c deps/brotli/c/enc/encoder_dict.c -o deps/brotli/c/enc/encoder_dict.o $(FLAGS)

deps/brotli/c/enc/entropy_encode.o: deps/brotli/c/enc/entropy_encode.c
	$(CC) -c deps/brotli/c/enc/entropy_encode.c -o deps/brotli/c/enc/entropy_encode.o $(FLAGS)

deps/brotli/c/enc/fast_log.o: deps/brotli/c/enc/fast_log.c
	$(CC) -c deps/brotli/c/enc/fast_log.c -o deps/brotli/c/enc/fast_log.o $(FLAGS)

deps/brotli/c/enc/histogram.o: deps/brotli/c/enc/histogram.c
	$(CC) -c deps/brotli/c/enc/histogram.c -o deps/brotli/c/enc/histogram.o $(FLAGS)

deps/brotli/c/enc/literal_cost.o: deps/brotli/c/enc/literal_cost.c
	$(CC) -c deps/brotli/c/enc/literal_cost.c -o deps/brotli/c/enc/literal_cost.o $(FLAGS)

deps/brotli/c/enc/memory.o: deps/brotli/c/enc/memory.c
	$(CC) -c deps/brotli/c/enc/memory.c -o deps/brotli/c/enc/memory.o $(FLAGS)

deps/brotli/c/enc/metablock.o: deps/brotli/c/enc/metablock.c
	$(CC) -c deps/brotli/c/enc/metablock.c -o deps/brotli/c/enc/metablock.o $(FLAGS)

deps/brotli/c/enc/static_dict.o: deps/brotli/c/enc/static_dict.c
	$(CC) -c deps/brotli/c/enc/static_dict.c -o deps/brotli/c/enc/static_dict.o $(FLAGS)

deps/brotli/c/enc/utf8_util.o: deps/brotli/c/enc/utf8_util.c
	$(CC) -c deps/brotli/c/enc/utf8_util.c -o deps/brotli/c/enc/utf8_util.o $(FLAGS)


deps/libz-static.a: deps/zlib/adler32.o deps/zlib/compress.o deps/zlib/crc32.o deps/zlib/deflate.o deps/zlib/gzclose.o deps/zlib/gzlib.o deps/zlib/gzread.o deps/zlib/gzwrite.o deps/zlib/infback.o deps/zlib/inffast.o deps/zlib/inflate.o deps/zlib/inftrees.o deps/zlib/trees.o deps/zlib/uncompr.o deps/zlib/zutil.o
	ar rvs deps/libz-static.a deps/temp/zlib/*.o

deps/zlib/adler32.o: deps/zlib/adler32.c
	$(CC) -c deps/zlib/adler32.c -o deps/zlib/adler32.o $(FLAGS)

deps/zlib/compress.o: deps/zlib/compress.c
	$(CC) -c deps/zlib/compress.c -o deps/zlib/compress.o $(FLAGS)

deps/zlib/crc32.o: deps/zlib/crc32.c
	$(CC) -c deps/zlib/crc32.c -o deps/zlib/crc32.o $(FLAGS)

deps/zlib/deflate.o: deps/zlib/deflate.c
	$(CC) -c deps/zlib/deflate.c -o deps/zlib/deflate.o $(FLAGS)

deps/zlib/gzclose.o: deps/zlib/gzclose.c
	$(CC) -c deps/zlib/gzclose.c -o deps/zlib/gzclose.o $(FLAGS)

deps/zlib/gzlib.o: deps/zlib/gzlib.c
	$(CC) -c deps/zlib/gzlib.c -o deps/zlib/gzlib.o $(FLAGS)

deps/zlib/gzread.o: deps/zlib/gzread.c
	$(CC) -c deps/zlib/gzread.c -o deps/zlib/gzread.o $(FLAGS)

deps/zlib/gzwrite.o: deps/zlib/gzwrite.c
	$(CC) -c deps/zlib/gzwrite.c -o deps/zlib/gzwrite.o $(FLAGS)

deps/zlib/infback.o: deps/zlib/infback.c
	$(CC) -c deps/zlib/infback.c -o deps/zlib/infback.o $(FLAGS)

deps/zlib/inffast.o: deps/zlib/inffast.c
	$(CC) -c deps/zlib/inffast.c -o deps/zlib/inffast.o $(FLAGS)

deps/zlib/inflate.o: deps/zlib/inflate.c
	$(CC) -c deps/zlib/inflate.c -o deps/zlib/inflate.o $(FLAGS)

deps/zlib/inftrees.o: deps/zlib/inftrees.c
	$(CC) -c deps/zlib/inftrees.c -o deps/zlib/inftrees.o $(FLAGS)

deps/zlib/trees.o: deps/zlib/trees.c
	$(CC) -c deps/zlib/trees.c -o deps/zlib/trees.o $(FLAGS)

deps/zlib/uncompr.o: deps/zlib/uncompr.c
	$(CC) -c deps/zlib/uncompr.c -o deps/zlib/uncompr.o $(FLAGS)

deps/zlib/zutil.o: deps/zlib/zutil.c
	$(CC) -c deps/zlib/zutil.c -o deps/zlib/zutil.o $(FLAGS)
