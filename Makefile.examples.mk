
example.kvserver: kvserver.example.exe

kvserver.example.exe: examples/kvserver.o $(LAMBDA_LIBSHARED)
	g++ -s $(CFLAGS) examples/kvserver.o $(LAMBDA_LIBSHARED) $(LINK_COMPRESS_LIBS) $(LINK_SYSTEM_LIBS) -o kvserver.example.exe

examples/kvserver.example.o: examples/kvserver.example.cpp
	g++ -c $(CFLAGS) examples/kvserver.example.cpp -o examples/kvserver.example.o


example.api_server: api_server.example.exe

api_server.example.exe: examples/api_server.o $(LAMBDA_LIBSHARED)
	g++ -s $(CFLAGS) examples/api_server.o $(LAMBDA_LIBSHARED) $(LINK_COMPRESS_LIBS) $(LINK_SYSTEM_LIBS) -o api_server.example.exe

examples/api_server.o: examples/api_server.cpp
	g++ -c $(CFLAGS) examples/api_server.cpp -o examples/api_server.o
