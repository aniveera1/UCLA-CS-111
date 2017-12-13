# NAME: Anirudh Veeraragavan

default: lab1a

lab1a: lab1a.c
	gcc -o lab1a -Wall -Wextra lab1a.c
	@echo "Executable created"

dist:
	@tar -cvzf lab1a-myid.tar.gz lab1a.c README Makefile

clean:
	@rm -f lab1a *.tar.gz *.txt
	@echo "All created files deleted"
