
# service page response example
example.servicepage: servicepage.example$(EXEEXT)

servicepage.example$(EXEEXT): examples/servicepage/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/servicepage/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o servicepage.example$(EXEEXT)

examples/servicepage/main.o: examples/servicepage/main.cpp
	g++ -c $(CFLAGS) examples/servicepage/main.cpp -o examples/servicepage/main.o
