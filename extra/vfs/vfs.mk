
LIB_EXTRA_VFS			=	extra/vfs.a
LIB_EXTRA_VFS_DEPS		=	extra/vfs/interface.o extra/vfs/syncqueue.o extra/vfs/format_tar.o

# storage stuff
$(LIB_EXTRA_VFS): $(LIB_EXTRA_VFS_DEPS)
	ar rvs $(LIB_EXTRA_VFS) $(LIB_EXTRA_VFS_DEPS)

extra/vfs/interface.o: extra/vfs/interface.cpp
	g++ -c $(CFLAGS) extra/vfs/interface.cpp -o extra/vfs/interface.o

extra/vfs/syncqueue.o: extra/vfs/syncqueue.cpp
	g++ -c $(CFLAGS) extra/vfs/syncqueue.cpp -o extra/vfs/syncqueue.o

extra/vfs/format_tar.o: extra/vfs/format_tar.cpp
	g++ -c $(CFLAGS) extra/vfs/format_tar.cpp -o extra/vfs/format_tar.o
