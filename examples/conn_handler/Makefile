
# extended request handler example
EXAMPLE_CONNHANDLER_TARGET = conn_handler.example$(EXEEXT)
example.conn_handler: $(EXAMPLE_CONNHANDLER_TARGET)

$(EXAMPLE_CONNHANDLER_TARGET): examples/conn_handler/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/conn_handler/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_CONNHANDLER_TARGET)

examples/conn_handler/main.o: examples/conn_handler/main.cpp
	g++ -c $(CFLAGS) examples/conn_handler/main.cpp -o examples/conn_handler/main.o
