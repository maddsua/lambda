
LIB_CORE_UTILS			=	core/utils.a
LIB_CORE_UTILS_DEPS		=	core/utils/byteswap.o

# lib utils
$(LIB_CORE_UTILS): $(LIB_CORE_UTILS_DEPS)
	ar rvs $(LIB_CORE_UTILS) $(LIB_CORE_UTILS_DEPS)

core/utils/byteswap.o: core/utils/byteswap.cpp
	g++ -c $(CFLAGS) core/utils/byteswap.cpp -o core/utils/byteswap.o
