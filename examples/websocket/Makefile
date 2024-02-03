
# websocket server example
EXAMPLE_WEBSOCKET_TARGET = websocket.example$(EXEEXT)
example.websocket: $(EXAMPLE_WEBSOCKET_TARGET)

$(EXAMPLE_WEBSOCKET_TARGET): examples/websocket/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/websocket/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_WEBSOCKET_TARGET)

examples/websocket/main.o: examples/websocket/main.cpp
	g++ -c $(CFLAGS) examples/websocket/main.cpp -o examples/websocket/main.o
