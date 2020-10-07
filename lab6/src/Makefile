all:
	make client
	make server

client:
	g++ -fsanitize=address client.cpp -o client.out /usr/local/lib/libzmq.a -pthread

server:
	g++ -fsanitize=address server.cpp -o server.out /usr/local/lib/libzmq.a -pthread

clean:
	rm -rf *.out
