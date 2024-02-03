
# sample api server
example.api_server: api_server.example$(EXEEXT)

api_server.example$(EXEEXT): examples/api_server/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/api_server/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o api_server.example$(EXEEXT)

examples/api_server/main.o: examples/api_server/main.cpp
	g++ -c $(CFLAGS) examples/api_server/main.cpp -o examples/api_server/main.o
