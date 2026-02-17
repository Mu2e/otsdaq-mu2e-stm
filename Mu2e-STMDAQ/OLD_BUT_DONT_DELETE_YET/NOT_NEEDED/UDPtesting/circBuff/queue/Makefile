CCFLAGS=-Wall -Wpedantic -Werror 
LDFLAGS = -lpq -pthread

test: test.cc queue.o | queue.hh
	g++ $(CCFLAGS) -o $@ $^ $(LDFLAGS)

queue.o: queue.cc | queue.hh 
	g++ $(CCFLAGS) -o $@ -c $^ $(LDFLAGS)

clean:
	rm -f queue.o test
