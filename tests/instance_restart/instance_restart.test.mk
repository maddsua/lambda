
# instance restart test
TEST_INSTANCE_RESTART_TARGET = instance_restart.test$(EXEEXT)
test.instance_restart: $(TEST_INSTANCE_RESTART_TARGET)
	$(TEST_INSTANCE_RESTART_TARGET)

$(TEST_INSTANCE_RESTART_TARGET): tests/instance_restart/instance_restart.test.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) tests/instance_restart/instance_restart.test.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(TEST_INSTANCE_RESTART_TARGET)

tests/instance_restart/instance_restart.test.o: tests/instance_restart/instance_restart.test.cpp
	g++ -c $(CFLAGS) tests/instance_restart/instance_restart.test.cpp -o tests/instance_restart/instance_restart.test.o
