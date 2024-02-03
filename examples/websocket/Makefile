
# websocket server example
example.websocket: websocket.example$(EXEEXT)

websocket.example$(EXEEXT): examples/websocket/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/websocket/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o websocket.example$(EXEEXT)

examples/websocket/main.o: examples/websocket/main.cpp
	g++ -c $(CFLAGS) examples/websocket/main.cpp -o examples/websocket/main.o
