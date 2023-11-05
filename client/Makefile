CC = g++
CFLAGS = -Wall
DEPS = ../Utilities/Headers.h ../Utilities/Models.h ../Utilities/Logger.h ../Utilities/Exceptions.h ../Utilities/Utilities.h
OBJ = client.o
%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

client: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lssl -lcrypto -pthread 
 
clean:
	rm -rf *o client
