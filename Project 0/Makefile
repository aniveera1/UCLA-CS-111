# NAME: Anirudh Veeraragavan

default: lab0

lab0: lab0.c
	gcc -o lab0 -Wall -Wextra lab0.c -g
	@echo "Executable created"

check:
	@echo "Running tests..."
	@echo "> Testing basic STDIN to STDOUT:"
	@echo This is a test. > test.txt
	@./lab0 < test.txt > output.txt
	@diff -q test.txt output.txt > /dev/null || \
	(echo "Test failed, exiting make..." && exit 1)
	@echo "> Testing --input to STDOUT:"
	@./lab0 --input=test.txt > output1.txt
	@diff -q test.txt output1.txt > /dev/null || \
	(echo "Test failed, exiting make..." && exit 1)
	@echo "> Testing STDIN to --output:"
	@./lab0 --output=output3.txt < test.txt
	@diff -q test.txt output3.txt > /dev/null || \
	(echo "Test failed, exiting make..." && exit 1)
	@echo "> Testing --input to --output:"
	@./lab0 --input=test.txt --output=output4.txt
	@ diff -q test.txt output4.txt > /dev/null || \
	(echo "Test failed, exiting make..." && exit 1)
	@echo "> Testing --segfault:"
	@./lab0 --segfault > /dev/null || if [[ $$? -ne 139 ]]; then echo "Test failed, exiting make..."; fi || true
	@echo "> Testing --segfault --catch:"
	@./lab0 --segfault --catch > /dev/null || if [[ $$? -ne 4 ]]; then echo "Test failed, exiting make..."; fi || true
	@echo "> Testing invalid --input:"
	@./lab0 --input=iamfake.txt > /dev/null || true
	@echo "> Testing all options:"
	@./lab0 --segfault --catch --input=test.txt --output=test.txt > /dev/null || if [[ $$? -ne 4 ]]; then echo "Test failed, exiting make..."; fi || true
	@echo "--------------------------------"
	@echo "All tests successful"

dist:
	@tar -cvzf lab0-myid.tar.gz lab0.c Makefile backtrace.png breakpoint.png README
	@echo "Distribution tarball created"

clean:
	@rm -f *.txt *.tar.gz lab0
	@echo "All created files deleted"
