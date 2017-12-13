# NAME: Anirudh Veeraragavan

build: lab4c_tcp.c lab4c_tls.c
	gcc -Wall -Wextra -lm -DDUMMY -o lab4c_tcp lab4c_tcp.c
	gcc -Wall -Wextra -lm -DDUMMY -lssl -lcrypto -o lab4c_tls lab4c_tls.c
	@echo "Executables created."

dist:
	@tar -cvzf lab4c-myid.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
	@echo "Distribution tarball created."

clean:
	@rm -rf Makefile~ README~ lab4c_tcp lab4c_tls lab4c-myid.tar.gz
	@echo "All created files deleted."