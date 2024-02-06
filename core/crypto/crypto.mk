
LIB_CORE_CRYPTO			=	core/crypto.a
LIB_CORE_CRYPTO_DEPS	=	core/crypto/sha1.o core/crypto/uid.o

# crypto stuff
$(LIB_CORE_CRYPTO): $(LIB_CORE_CRYPTO_DEPS)
	ar rvs $(LIB_CORE_CRYPTO) $(LIB_CORE_CRYPTO_DEPS)

core/crypto/sha1.o: core/crypto/sha1.cpp
	g++ -c $(CFLAGS) core/crypto/sha1.cpp -o core/crypto/sha1.o

core/crypto/uid.o: core/crypto/uid.cpp
	g++ -c $(CFLAGS) core/crypto/uid.cpp -o core/crypto/uid.o
