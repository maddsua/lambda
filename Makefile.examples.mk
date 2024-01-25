
example.api_server: api_server.example$(EXEEXT)

api_server.example$(EXEEXT): examples/api_server.o $(LAMBDA_LIBSHARED)
	g++ -s $(CFLAGS) examples/api_server.o $(LAMBDA_LIBSHARED) $(LINK_OTHER_LIBS) $(LINK_SYSTEM_LIBS) -o api_server.example$(EXEEXT)

examples/api_server.o: examples/api_server.cpp
	g++ -c $(CFLAGS) examples/api_server.cpp -o examples/api_server.o

example.kvserver: kvserver.example$(EXEEXT)

kvserver.example$(EXEEXT): examples/kvserver.o $(LAMBDA_LIBSHARED)
	g++ -s $(CFLAGS) examples/kvserver.o $(LAMBDA_LIBSHARED) $(LINK_OTHER_LIBS) $(LINK_SYSTEM_LIBS) -o kvserver.example$(EXEEXT)

examples/kvserver.example.o: examples/kvserver.example.cpp
	g++ -c $(CFLAGS) examples/kvserver.example.cpp -o examples/kvserver.example.o
