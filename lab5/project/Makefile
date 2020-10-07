LIBRARY_PATH=/home/zero/documents/mai/os/library
MAIN_PATH=/home/zero/documents/mai/os/project

LIBRARY_COMPILE_FLAGS=-Wall
MAIN_COMPILE_FLAGS=-Wall
SANITIZE_FLAGS=-fsanitize=address -fsanitize=undefined

LIB_NAME=test

PROGRAM_NAME=program

first:
	gcc $(LIBRARY_COMPILE_FLAGS) -c -fPIC $(LIBRARY_PATH)/source/*.c
	gcc -shared -o $(LIBRARY_PATH)/lib$(LIB_NAME).so *.o
	gcc $(MAIN_COMPILE_FLAGS) -c $(MAIN_PATH)/main1.c
	gcc $(SANITIZE_FLAGS) -o $(PROGRAM_NAME) *.o -L$(LIBRARY_PATH) -l$(LIB_NAME) -Wl,-rpath,$(LIBRARY_PATH)

second:
	gcc $(LIBRARY_COMPILE_FLAGS) -c -fPIC $(LIBRARY_PATH)/source/*.c
	gcc -shared -o $(LIBRARY_PATH)/lib$(LIB_NAME).so *.o
	gcc $(MAIN_COMPILE_FLAGS) -o $(PROGRAM_NAME) $(MAIN_PATH)/main2.c -ldl

clean:
	rm *.o
