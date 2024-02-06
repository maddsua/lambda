
# sample api server
EXAMPLE_HELLOWORLD_TARGET = helloworld.example$(EXEEXT)
example.helloworld: $(EXAMPLE_HELLOWORLD_TARGET)

$(EXAMPLE_HELLOWORLD_TARGET): examples/helloworld/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/helloworld/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_HELLOWORLD_TARGET)

examples/helloworld/main.o: examples/helloworld/main.cpp
	g++ -c $(CFLAGS) examples/helloworld/main.cpp -o examples/helloworld/main.o
