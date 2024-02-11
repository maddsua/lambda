
LIB_CORE_SSE			=	core/sse.a
LIB_CORE_SSE_DEPS		=	core/sse/writer.o

# server side events stuff
$(LIB_CORE_SSE): $(LIB_CORE_SSE_DEPS)
	ar rvs $(LIB_CORE_SSE) $(LIB_CORE_SSE_DEPS)

core/sse/writer.o: core/sse/writer.cpp
	g++ -c $(CFLAGS) core/sse/writer.cpp -o core/sse/writer.o
