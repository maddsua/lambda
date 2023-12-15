
example.kvserver: kvserver.example.exe

kvserver.example.exe: examples/kvserver.example.o $(LAMBDA_LIBSTATIC)
	g++ $(CFLAGS) examples/kvserver.example.o $(LAMBDA_LIBSTATIC) $(LINK_COMPRESS_LIBS) $(LINK_SYSTEM_LIBS) -o kvserver.example.exe

examples/kvserver.example.o: examples/kvserver.example.cpp
	g++ -c $(CFLAGS) examples/kvserver.example.cpp -o examples/kvserver.example.o
