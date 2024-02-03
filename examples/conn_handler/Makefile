
# extended request handler example
example.conn_handler: conn_handler.example$(EXEEXT)

conn_handler.example$(EXEEXT): examples/conn_handler/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/conn_handler/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o conn_handler.example$(EXEEXT)

examples/conn_handler/main.o: examples/conn_handler/main.cpp
	g++ -c $(CFLAGS) examples/conn_handler/main.cpp -o examples/conn_handler/main.o
