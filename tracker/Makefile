CC = g++
CFLAGS = -Wall
DEPS = ../Utilities/Headers.h ../Utilities/Models.h ../Utilities/Logger.h ../Utilities/Exceptions.h ../Utilities/Utilities.h
OBJ = tracker.o
%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

tracker: $(OBJ)
	$(CC) $(CFLAGS) -pthread -o $@ $^ 

clean:
	rm -rf *o tracker
