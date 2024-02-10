
LIB_CORE_UTILS			=	core/utils.a
LIB_CORE_UTILS_DEPS		=	core/utils/byteswap.o core/utils/os_error.o core/utils/syncout.o

# lib utils
$(LIB_CORE_UTILS): $(LIB_CORE_UTILS_DEPS)
	ar rvs $(LIB_CORE_UTILS) $(LIB_CORE_UTILS_DEPS)

core/utils/byteswap.o: core/utils/byteswap.cpp
	g++ -c $(CFLAGS) core/utils/byteswap.cpp -o core/utils/byteswap.o

core/utils/os_error.o: core/utils/os_error.cpp
	g++ -c $(CFLAGS) core/utils/os_error.cpp -o core/utils/os_error.o

core/utils/syncout.o: core/utils/syncout.cpp
	g++ -c $(CFLAGS) core/utils/syncout.cpp -o core/utils/syncout.o
