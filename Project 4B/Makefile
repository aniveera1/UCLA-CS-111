# NAME: Anirudh Veeraragavan
# EMAIL: aveeraragavan@g.ucla.edu
# ID: 004767663

build: lab4b.c
	gcc -Wall -lm -Wextra -lmraa -o lab4b lab4b.c;
	@echo "Executable created"

check:
	@printf "SCALE=F\nPERIOD=1\nSTART\nSTOP\nOFF" | ./lab4b --period=2 --log="LOGFILE"
	@grep -oE '[0-9]+\.[0-9]\>' LOGFILE > /dev/null || \
	(echo "Smoke Test Failed: A temperature was not reported" && exit 1)
	@grep -oE 'SCALE' LOGFILE > /dev/null || \
	(echo "Smoke Test Failed: SCALE was not reported" && exit 1)
	@grep -oE 'PERIOD' LOGFILE > /dev/null || \
	(echo "Smoke Test Failed: PERIOD was not reported" && exit 1)
	@grep -oE 'STOP' LOGFILE > /dev/null || \
	(echo "Smoke Test Failed: STOP was not reported" && exit 1)
	@grep -oE 'START' LOGFILE > /dev/null || \
	(echo "Smoke Test Failed: START was not reported" && exit 1)
	@grep -oE 'OFF' LOGFILE > /dev/null || \
	(echo "Smoke Test Failed: OFF was not reported" && exit 1)
	@rm -f LOGFILE
	@echo "Smoke Test Passed"

dist:
	@tar -cvzf lab4b-004767663.tar.gz lab4b.c Makefile README
	@echo "Distribution tarball created"

clean:
	@rm -rf Makefile~ lab4b.c~ lab4b lab4b-004767663.tar.gz
	@echo "All created files deleted"
