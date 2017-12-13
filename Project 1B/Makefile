# NAME: Anirudh Veeraragavan

default: lab1b-client lab1b-server

client: lab1b-client

server: lab1b-server

lab1b-client: lab1b-client.c
	gcc -o lab1b-client -Wall -Wextra -lmcrypt lab1b-client.c
	@echo "Client executable created"

lab1b-server: lab1b-server.c
	gcc -o lab1b-server -Wall -Wextra -lmcrypt lab1b-server.c
	@echo "Server executable created"

clean:
	@rm -f lab1b-client lab1b-server lab1b-myid.tar.gz *.txt
	@echo "All created files deleted"

dist:
	@tar -cvzf lab1b-myid.tar.gz lab1b-client.c lab1b-server.c Makefile my.key README
	@echo "Distribution tarball created"
