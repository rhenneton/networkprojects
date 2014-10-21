all : server.c client.c
	gcc server.c -o server
	gcc client.c -o client
	
clean : server.c client.c
	rm client
	rm server
	
	
