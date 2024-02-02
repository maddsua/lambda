
# service page response example
example.servicepage: servicepage.example$(EXEEXT)

servicepage.example$(EXEEXT): examples/servicepage.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/servicepage.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o servicepage.example$(EXEEXT)

examples/servicepage.o: examples/servicepage.cpp
	g++ -c $(CFLAGS) examples/servicepage.cpp -o examples/servicepage.o


# service crash example
example.crash: crash.example$(EXEEXT)

crash.example$(EXEEXT): examples/crash.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/crash.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o crash.example$(EXEEXT)

examples/crash.o: examples/crash.cpp
	g++ -c $(CFLAGS) examples/crash.cpp -o examples/crash.o


# sample api server
example.api_server: api_server.example$(EXEEXT)

api_server.example$(EXEEXT): examples/api_server.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/api_server.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o api_server.example$(EXEEXT)

examples/api_server.o: examples/api_server.cpp
	g++ -c $(CFLAGS) examples/api_server.cpp -o examples/api_server.o


# sample kv storage server
example.kvserver: kvserver.example$(EXEEXT)

kvserver.example$(EXEEXT): examples/kvserver.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/kvserver.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o kvserver.example$(EXEEXT)

examples/kvserver.o: examples/kvserver.cpp
	g++ -c $(CFLAGS) examples/kvserver.cpp -o examples/kvserver.o


# extended request handler example
example.conn_handler: conn_handler.example$(EXEEXT)

conn_handler.example$(EXEEXT): examples/conn_handler.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/conn_handler.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o conn_handler.example$(EXEEXT)

examples/conn_handler.o: examples/conn_handler.cpp
	g++ -c $(CFLAGS) examples/conn_handler.cpp -o examples/conn_handler.o


# websocket server example
example.websocket: websocket.example$(EXEEXT)

websocket.example$(EXEEXT): examples/websocket.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/websocket.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o websocket.example$(EXEEXT)

examples/websocket.o: examples/websocket.cpp
	g++ -c $(CFLAGS) examples/websocket.cpp -o examples/websocket.o
