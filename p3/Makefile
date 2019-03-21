objects = main.o server.o connection_manager.o connection.o request_parser.o request_handler.o

all: console.cgi http_server

console.cgi: console.cpp
	clang++ console.cpp -o console.cgi -I/usr/local/include -L/usr/local/lib -std=c++11 -Wall -pedantic -pthread -lboost_system

http_server: $(objects)
	clang++ $(objects) -o http_server -I/usr/local/include -L/usr/local/lib -std=c++11 -Wall -pedantic -pthread -lboost_system

main.o: main.cpp server.hpp
	clang++ main.cpp -I/usr/local/include -std=c++11 -Wall -pedantic -pthread -c
	
server.o: server.cpp server.hpp
	clang++ server.cpp -I/usr/local/include -std=c++11 -Wall -pedantic -pthread -c
	
connection_manager.o: connection_manager.cpp connection_manager.hpp
	clang++ connection_manager.cpp -I/usr/local/include -std=c++11 -Wall -pedantic -pthread -c
	
connection.o: connection.cpp connection.hpp connection_manager.hpp request_handler.hpp
	clang++ connection.cpp -I/usr/local/include -std=c++11 -Wall -pedantic -pthread -c
	
request_parser.o: request_parser.cpp request_parser.hpp request.hpp
	clang++ request_parser.cpp -I/usr/local/include -std=c++11 -Wall -pedantic -pthread -c
	
request_handler.o: request_handler.cpp request_handler.hpp reply.hpp request.hpp
	clang++ request_handler.cpp -I/usr/local/include -std=c++11 -Wall -pedantic -pthread -c







	
