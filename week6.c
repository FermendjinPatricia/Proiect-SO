#include <stdio.h>
#include <string.h>
#include <error.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

struct stat informatii;

void testArgs(int argc, char *argv[]){
    if (argc != 2){
        printf("Usage %s\n",argv[1]);
        exit(-1);
    }
    const char *ext = strrchr(argv[1], '.');
    if (ext == NULL || strcmp(ext, ".bmp") != 0){
        printf("Usage %s %s\n",argv[0],argv[1]);
        exit(-1);
    }
}

int openFile(char *name){
    int fin=open(name, O_RDONLY);
    if(fin==-1){
        perror("Nu s-a putut deschide fisierul.\n");
        exit(-1);
    }
    return fin;
}

int createFile(char *name){
    int fout=creat(name, S_IRUSR | S_IWUSR | S_IXUSR);
    if(fout==-1){
        perror("Nu s-a putut crea fisierul.\n");
        exit(-1);
    }
    return fout;
}

/*int writeInFile(int fileDescriptor, char *buffer, int charBytes){
    int w = write(fileDescriptor,buffer,charBytes);
    if( w == -1 ){
        perror("Nu s-a putut scrie numele fisierului.\n");
        exit(-1);
    }
    return w;
}

int writeInBuffer(char *buffer, char *message, char *s){
    return sprintf(buffer,message,s);
}*/

void writeName(int fileDescriptorOut, char *nume){
    char buffer[100];
    int charBytes = sprintf(buffer,"nume fisier: %s\n",nume);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-a putut scrie numele fisierului.\n");
        exit(-1);
    }
}

int getFilesWidth(int fileDescriptorIn){
    int width;
    lseek(fileDescriptorIn,18, SEEK_CUR);
    if(read(fileDescriptorIn, &width, 4)==-1){
        perror("Nu s-a realizat corect citirea inaltimii.\n");
        exit(-1);
    }
    return width;
}

int getFilesHeight(int fileDescriptorIn){
    int height;
    if(read(fileDescriptorIn,&height,4)==-1){
        perror("Nu s-a realizat corect citirea lungimii.\n");
        exit(-1);
    }
    return height;
}

void writeWidth(int fileDescriptorIn, int fileDescriptorOut){
    int width = getFilesWidth(fileDescriptorIn);
    char buffer[100];
    int charBytes = sprintf(buffer,"inaltime: %d\n", width);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-a putut scrie inaltimea.\n");
        exit(-1);
    }
}

void writeHeight(int fileDescriptorIn, int fileDescriptorOut){
    int height = getFilesHeight(fileDescriptorIn);
    char buffer[100];
    int charBytes = sprintf(buffer,"lungime: %d\n", height);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-a putut scrie lungimea.\n");
        exit(-1);
    }
}

void writeSize(int fileDescriptorOut, int size){
    char buffer[100];
    int charBytes = sprintf(buffer,"dimensiune: %d\n",size);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-a putut scrie dimensiunea.\n");
        exit(-1);
    }
}

void writeUserId(int fileDescriptorOut, uid_t userId){
    char buffer[100];
    int charBytes = sprintf(buffer,"identificatorul utilizatorului: %u\n",userId);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-a putut scrie identificatorul utilizatorului.\n");
        exit(-1);
    }
}

typedef struct Date {
   short day, month, year;
} DATE;

DATE timeTConverter(time_t date) 
{
    struct tm *giverTime;
    giverTime = localtime(&date); /* pass a pointer */
    DATE given;
    given.day = giverTime->tm_mday;
    given.month = giverTime->tm_mon+1; /* simply add instead of increment */
    given.year = giverTime->tm_year+1900;

    return given;
}

void writeTime(int fileDescriptorOut, time_t time){
    DATE date = timeTConverter(time);
    char buffer[100];
    int charBytes = sprintf(buffer,"timpul ultimei modificari: %d.%d.%d\n", date.day, date.month, date.year);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-a putut scrie identificatorul utilizatorului.\n");
        exit(-1);
    }
}

void writeHardLinks(int fileDescriptorOut, nlink_t links){
    char buffer[100];
    int charBytes = sprintf(buffer,"contorul de legaturi: %ld\n", links);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-a putut scrie contorul de legaturi.\n");
        exit(-1);
    }
}

void getUserAccessRights(mode_t mode, char *buffer){
    char readRights = (mode & S_IRUSR) ? 'r' : '-' ;
    char writeRights = (mode & S_IWUSR) ? 'w' : '-' ;
    char executeRights = (mode & S_IXUSR) ? 'x' : '-';

    sprintf(buffer,"%c%c%c",readRights,writeRights,executeRights);
}

void writeUserAccessRights(int fileDescriptorOut, mode_t mode){
    char buffer[100],bufferRights[50];
    getUserAccessRights(mode,bufferRights);
    int charBytes = sprintf(buffer,"drepturi de acces user: %s\n", bufferRights);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-au putut scrie drepturile de acces pentru user.\n");
        exit(-1);
    }
}

void getGroupAccessRights(mode_t mode, char *buffer){
    char readRights = (mode & S_IRGRP) ? 'r' : '-';
    char writeRights = (mode & S_IWGRP) ? 'w' : '-';
    char executeRights = (mode & S_IXGRP) ? 'x' : '-';

    sprintf(buffer,"%c%c%c",readRights,writeRights,executeRights);
}

void writeGroupAccessRights(int fileDescriptorOut, mode_t mode){
    char buffer[100],bufferRights[50];
    getGroupAccessRights(mode,bufferRights);
    int charBytes = sprintf(buffer,"drepturi de acces grup: %s\n", bufferRights);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-au putut scrie drepturile de acces pentru grup.\n");
        exit(-1);
    }
}

void getOthersAccessRights(mode_t mode, char *buffer){
    char readRights = (mode & S_IROTH) ? 'r' : '-';
    char writeRights = (mode & S_IWOTH) ? 'w' : '-';
    char executeRights = (mode & S_IXOTH) ? 'x' : '-';

    sprintf(buffer,"%c%c%c",readRights,writeRights,executeRights);
}

void writeOthersAccessRights(int fileDescriptorOut, mode_t mode){
    char buffer[100],bufferRights[50];
    getOthersAccessRights(mode,bufferRights);
    int charBytes = sprintf(buffer,"drepturi de acces grup: %s\n", bufferRights);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-au putut scrie drepturile de acces pentru grup.\n");
        exit(-1);
    }
}

int main(int argc,char *argv[]){
    testArgs(argc,argv);
    int fileDescriptorIn = openFile(argv[1]);
    int fileDescriptorOut = createFile("statistica.txt");

    int stat_out=stat(argv[1],&informatii);
    if(stat_out==-1)
    {
        perror("S-a produs o eroare la aflarea atributelor.");
        exit(-1);
    }
    writeName(fileDescriptorOut, argv[1]);
    writeWidth(fileDescriptorIn, fileDescriptorOut);
    writeHeight(fileDescriptorIn, fileDescriptorOut);
    writeSize(fileDescriptorOut,  informatii.st_size);
    writeUserId(fileDescriptorOut, informatii.st_uid);
    writeTime(fileDescriptorOut, informatii.st_mtime);
    writeHardLinks(fileDescriptorOut,informatii.st_nlink);
    writeUserAccessRights(fileDescriptorOut,informatii.st_mode);
    writeGroupAccessRights(fileDescriptorOut,informatii.st_mode);
    writeOthersAccessRights(fileDescriptorOut,informatii.st_mode);
    return 0;
}