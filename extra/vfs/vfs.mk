
LIB_EXTRA_VFS			=	extra/vfs.a
LIB_EXTRA_VFS_DEPS		=	extra/vfs/interface.o

# storage stuff
$(LIB_EXTRA_VFS): $(LIB_EXTRA_VFS_DEPS)
	ar rvs $(LIB_EXTRA_VFS) $(LIB_EXTRA_VFS_DEPS)

extra/vfs/interface.o: extra/vfs/interface.cpp
	g++ -c $(CFLAGS) extra/vfs/interface.cpp -o extra/vfs/interface.o
