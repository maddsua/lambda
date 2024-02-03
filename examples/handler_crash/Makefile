
# handler crash example
EXAMPLE_HANDLERCRASH_TARGET = handler_crash.example$(EXEEXT)
example.handler_crash: $(EXAMPLE_HANDLERCRASH_TARGET)

$(EXAMPLE_HANDLERCRASH_TARGET): examples/handler_crash/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/handler_crash/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_HANDLERCRASH_TARGET)

examples/handler_crash/main.o: examples/handler_crash/main.cpp
	g++ -c $(CFLAGS) examples/handler_crash/main.cpp -o examples/handler_crash/main.o
