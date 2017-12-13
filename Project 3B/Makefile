# NAME: Anirudh Veeraragavan
# EMAIL: aveeraragavan@g.ucla.edu
# ID: 004767663

build: lab3b.py
	@chmod u+x lab3b.py
	@ln -s lab3b.py lab3b
	@echo "Executable lab3b created."

dist:
	@tar -cvzf lab3b-004767663.tar.gz lab3b.py Makefile README
	@echo "Distribution tarball created."

clean:
	@rm -f Makefile~ README~ lab3b lab3b-004767663.tar.gz
	@echo "All created files deleted."