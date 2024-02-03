
# sample kv storage server
example.kvserver: kvserver.example$(EXEEXT)

kvserver.example$(EXEEXT): examples/kvserver/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/kvserver/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o kvserver.example$(EXEEXT)

examples/kvserver/main.o: examples/kvserver/main.cpp
	g++ -c $(CFLAGS) examples/kvserver/main.cpp -o examples/kvserver/main.o
