EXTRA_OBJ_TARGET	=	extra/extra.o
EXTRA_OBJ_DEPS		=	$(OBJ_JSON)

OBJ_JSON			=	extra/json/property.o extra/json/parse.o extra/json/stringify.o

# extra object
octo.core: $(EXTRA_OBJ_TARGET)

$(EXTRA_OBJ_TARGET): $(OBJ_JSON)
	ld -relocatable $(OBJ_JSON) -o $(EXTRA_OBJ_TARGET)

# jisson stuff
extra/json/parse.o: extra/json/parse.cpp
	g++ -c extra/json/parse.cpp -o extra/json/parse.o $(CFLAGS)

extra/json/stringify.o: extra/json/stringify.cpp
	g++ -c extra/json/stringify.cpp -o extra/json/stringify.o $(CFLAGS)

extra/json/property.o: extra/json/property.cpp
	g++ -c extra/json/property.cpp -o extra/json/property.o $(CFLAGS)
