CFLAGS	= -g -Wall
CC	= g++ $(CFLAGS)

all:
	$(CC) -o Thread_Pool Thread_Pool_Tasks.cpp 

clean:
	rm -rf Thread_Pool
