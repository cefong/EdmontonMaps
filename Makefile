CC=g++
OBJS= digraph.o dijkstra.o server.o
CFLAGS=-c -Wall -O2
LFLAGS=-static
PROGRAM=server

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM) $(LFLAGS)

digraph.o: digraph.cpp
	$(CC) digraph.cpp -o digraph.o $(CFLAGS)

dijkstra.o: dijkstra.cpp dijkstra.h
	$(CC) dijkstra.cpp -o dijkstra.o $(CFLAGS)

server.o: server.cpp
	$(CC) server.cpp -o server.o $(CFLAGS)	
clean:
	@rm -f $(OBJS)
	@rm -f $(PROGRAM)