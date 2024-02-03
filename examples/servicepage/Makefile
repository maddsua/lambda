
# service page response example
EXAMPLE_SERVICEPAGE_TARGET = servicepage.example$(EXEEXT)
example.servicepage: $(EXAMPLE_SERVICEPAGE_TARGET)

$(EXAMPLE_SERVICEPAGE_TARGET): examples/servicepage/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/servicepage/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_SERVICEPAGE_TARGET)

examples/servicepage/main.o: examples/servicepage/main.cpp
	g++ -c $(CFLAGS) examples/servicepage/main.cpp -o examples/servicepage/main.o
