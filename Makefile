out.exe: stat
	gcc -Wall -o proiect  proiect.c

stat:
	rm -r directorIesire
	mkdir directorIesire
	