
# handler crash example
example.handler_crash: handler_crash.example$(EXEEXT)

handler_crash.example$(EXEEXT): examples/handler_crash/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/handler_crash/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o handler_crash.example$(EXEEXT)

examples/handler_crash/main.o: examples/handler_crash/main.cpp
	g++ -c $(CFLAGS) examples/handler_crash/main.cpp -o examples/handler_crash/main.o
