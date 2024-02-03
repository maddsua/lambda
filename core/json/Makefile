
LIB_CORE_JSON			=	core/json.a
LIB_CORE_JSON_DEPS		=	core/json/property.o core/json/parse.o core/json/stringify.o

# jisson stuff
$(LIB_CORE_JSON): $(LIB_CORE_JSON_DEPS)
	ar rvs $(LIB_CORE_JSON) $(LIB_CORE_JSON_DEPS)

core/json/parse.o: core/json/parse.cpp
	g++ -c $(CFLAGS) core/json/parse.cpp -o core/json/parse.o

core/json/stringify.o: core/json/stringify.cpp
	g++ -c $(CFLAGS) core/json/stringify.cpp -o core/json/stringify.o

core/json/property.o: core/json/property.cpp
	g++ -c $(CFLAGS) core/json/property.cpp -o core/json/property.o
