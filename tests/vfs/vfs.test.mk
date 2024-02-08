
# Test vfs extra module
TEST_VFS_TARGET = $(EXEPFX)vfs.test$(EXEEXT)
TEST_VFS_DEPS	= $(LIB_CORE_POLYFILL_DEPS) $(LIB_EXTRA_VFS_DEPS) $(LIB_CORE_COMPRESS)

test.vfs: $(TEST_VFS_TARGET)
	$(TEST_VFS_TARGET)

$(TEST_VFS_TARGET): tests/vfs/main.cpp $(TEST_VFS_DEPS)
	g++ $(CFLAGS) tests/vfs/main.cpp $(TEST_VFS_DEPS) -lz -o $(TEST_VFS_TARGET)
