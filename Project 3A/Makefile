# NAME: Anirudh Veeraragavan,Rahul Sheth

build: lab3a.c
		gcc -o lab3a -Wall -Wextra lab3a.c
		@echo "Executable created."

dist:
	@tar -cvzf lab3a-myid.tar.gz lab3a.c Makefile README
	@echo "Distribution tarball created."

clean:
	@rm -f Makefile~ README~ lab3a lab3a-myid.tar.gz
	@echo "All created files deleted."