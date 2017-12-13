# NAME: Anirudh Veeraragavan

build: lab2_list.c SortedList.c
	gcc -o lab2_list -Wall -Wextra -pthread lab2_list.c SortedList.c
	@echo "Executable created"

tests:
	@rm -f lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=2 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=2 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=4 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=4 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=8 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=8 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=16 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=16 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=2 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=2 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=4 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=4 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=8 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=8 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=16 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=16 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=2 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=2 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=4 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=4 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=8 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=8 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=16 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=16 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=2 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=2 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=4 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=4 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=8 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=8 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=16 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=16 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=2 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=2 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=4 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=4 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=8 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=8 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=16 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=16 --lists=4 --yield=id --sync=s >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

	@echo "CSV file generated"

profile:
	@make clean
	@gcc -o lab2_list -Wall -Wextra -pthread -g -lprofiler lab2_list.c SortedList.c
	@CPUPROFILE=~/profile.out ./lab2_list --iterations=1000 --threads=12 --sync=s
	@echo "Profiling report generated"

graphs:
	@./lab2b.gp
	@echo "All graphs generated"

dist:
	@make profile
	@make clean
	@make
	@make tests
	@make graphs
	@tar -cvzf lab2b-myid.tar.gz SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b.gp README

clean:
	@rm -f Makefile~ README~ testThreads.sh~ lab2_list lab2b-myid.tar.gz
	@echo "All created files deleted"
