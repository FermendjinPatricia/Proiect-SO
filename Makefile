out.exe: stat
	gcc -Wall -o p week6.c

stat:
	rm -f statistica.txt