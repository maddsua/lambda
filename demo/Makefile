
APP      = lambda_test.exe
MAIN     = main
OBJECTS  = main.o
FLAGS    = -std=c++20
LIBS     = -lmda -lws2_32 -lz -lbrotlienc -lbrotlidec

.PHONY: all all-before all-after clean clean-custom run-custom

all: all-before $(APP) all-after

clean: clean-custom
	del /S *.o *.exe

run: run-custom
	$(APP)

$(APP): $(OBJECTS)
	g++ $(OBJECTS) -o $(APP) $(LIBS) -L"../lib/"

$(MAIN).o: $(MAIN).cpp
	g++ -c $(MAIN).cpp -o $(MAIN).o $(FLAGS)
