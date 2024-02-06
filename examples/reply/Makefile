
# sample api server
EXAMPLE_REPLY_TARGET = reply.example$(EXEEXT)
example.reply: $(EXAMPLE_REPLY_TARGET)

$(EXAMPLE_REPLY_TARGET): examples/reply/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/reply/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_REPLY_TARGET)

examples/reply/main.o: examples/reply/main.cpp
	g++ -c $(CFLAGS) examples/reply/main.cpp -o examples/reply/main.o
