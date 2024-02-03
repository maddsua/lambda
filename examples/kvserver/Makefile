
# sample kv storage server
EXAMPLE_KVSERVER_TARGET = kvserver.example$(EXEEXT)
example.kvserver: $(EXAMPLE_KVSERVER_TARGET)

$(EXAMPLE_KVSERVER_TARGET): examples/kvserver/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/kvserver/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_KVSERVER_TARGET)

examples/kvserver/main.o: examples/kvserver/main.cpp
	g++ -c $(CFLAGS) examples/kvserver/main.cpp -o examples/kvserver/main.o
