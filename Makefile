compile: server.cpp client.cpp
	g++ -std=c++11 -fpermissive -o client client.cpp
	g++ -std=c++11 -fpermissive -pthread -o server server.cpp

clean:
	rm server client
