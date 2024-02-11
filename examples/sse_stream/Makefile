
# sse stream example
EXAMPLE_SSESTREAM_TARGET = sse_stream.example$(EXEEXT)
example.sse_stream: $(EXAMPLE_SSESTREAM_TARGET)

$(EXAMPLE_SSESTREAM_TARGET): examples/sse_stream/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/sse_stream/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_SSESTREAM_TARGET)

examples/sse_stream/main.o: examples/sse_stream/main.cpp
	g++ -c $(CFLAGS) examples/sse_stream/main.cpp -o examples/sse_stream/main.o
