EXTRA_TARGET			=	extra/extra.a
EXTRA_DEPS				=	$(EXTRA_JSON_DEPS) $(EXTRA_STORAGE_DEPS)

EXTRA_JSON_TARGET		=	extra/json.a
EXTRA_JSON_DEPS			=	extra/json/property.o extra/json/parse.o extra/json/stringify.o

EXTRA_STORAGE_TARGET	=	extra/storage.a
EXTRA_STORAGE_DEPS		=	extra/storage/storage.o extra/storage/localstorage.o


# extra object
lambda.extra: $(EXTRA_TARGET)

$(EXTRA_TARGET): $(EXTRA_DEPS)
	ar rvs $(EXTRA_TARGET) $(EXTRA_DEPS)


# jisson stuff
$(EXTRA_JSON_TARGET): $(EXTRA_JSON_DEPS)
	ar rvs $(EXTRA_JSON_TARGET) $(EXTRA_JSON_DEPS)

extra/json/parse.o: extra/json/parse.cpp
	g++ -c $(CFLAGS) extra/json/parse.cpp -o extra/json/parse.o

extra/json/stringify.o: extra/json/stringify.cpp
	g++ -c $(CFLAGS) extra/json/stringify.cpp -o extra/json/stringify.o

extra/json/property.o: extra/json/property.cpp
	g++ -c $(CFLAGS) extra/json/property.cpp -o extra/json/property.o


# storage stuff
$(EXTRA_STORAGE_TARGET): $(EXTRA_STORAGE_DEPS)
	ar rvs $(EXTRA_STORAGE_TARGET) $(EXTRA_STORAGE_DEPS)

extra/storage/storage.o: extra/storage/storage.cpp
	g++ -c $(CFLAGS) extra/storage/storage.cpp -o extra/storage/storage.o

extra/storage/localstorage.o: extra/storage/localstorage.cpp
	g++ -c $(CFLAGS) extra/storage/localstorage.cpp -o extra/storage/localstorage.o
