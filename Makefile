out.exe: stat
	gcc -Wall -o test  proiect.c

stat:
	rm -r directorOut
	mkdir directorOut
	