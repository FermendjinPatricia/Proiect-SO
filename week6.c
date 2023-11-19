#include <stdio.h>
#include <string.h>
#include <error.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

struct stat informatii;

typedef enum type_of_file{
    REGULAR,DIRECTOR,LINK
} type_of_file;

void testArgs(int argc, char *argv[]){
    if (argc != 2){
        printf("Usage %s\n",argv[1]);
        exit(-1);
    }
}

int checkBMPFile(char *name){
    const char *ext = strrchr(name, '.');
    if (ext == NULL || strcmp(ext, ".bmp") != 0){
        return 0;
    }
    return 1;
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
    int fout=open(name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
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

void writeName(int fileDescriptorOut, char *nume, char *type){
    char buffer[100];
    int charBytes = sprintf(buffer,"nume %s: %s\n",type, nume);
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

void writeSize(int fileDescriptorOut, int size, char *type){
    char buffer[100];
    int charBytes = sprintf(buffer,"dimensiune%s: %d\n", type, size);
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

void writeUserAccessRights(int fileDescriptorOut, mode_t mode, char *type){
    char buffer[100],bufferRights[50];
    getUserAccessRights(mode,bufferRights);
    int charBytes = sprintf(buffer,"drepturi de acces user%s: %s\n",type, bufferRights);
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

void writeGroupAccessRights(int fileDescriptorOut, mode_t mode, char *type){
    char buffer[100],bufferRights[50];
    getGroupAccessRights(mode,bufferRights);
    int charBytes = sprintf(buffer,"drepturi de acces grup%s: %s\n",type, bufferRights);
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

void writeOthersAccessRights(int fileDescriptorOut, mode_t mode, char *type){
    char buffer[100],bufferRights[50];
    getOthersAccessRights(mode,bufferRights);
    int charBytes = sprintf(buffer,"drepturi de acces altii%s: %s\n",type, bufferRights);
    if((write(fileDescriptorOut,buffer,charBytes)) == -1){
        perror("Nu s-au putut scrie drepturile de acces pentru altii.\n");
        exit(-1);
    }
}
type_of_file typeOfFile(mode_t mode){
    if(S_ISREG(mode)!=0) return REGULAR; //regular file
    if(S_ISDIR(mode)!=0) return DIRECTOR; // director
    if(S_ISLNK(mode)!=0) return LINK; //link
    
    return -1;
    
}

void writeStatisticsByType(type_of_file type, char *name){
    switch (type)
    {
    case REGULAR:{
            int fileDescriptorOut = createFile("statistica.txt");
            int fileDescriptorIn = openFile(name);
            writeName(fileDescriptorOut, name,"fisier");
            if(checkBMPFile(name)==1){
                writeWidth(fileDescriptorIn, fileDescriptorOut);
                writeHeight(fileDescriptorIn, fileDescriptorOut);
            }
            writeSize(fileDescriptorOut,  informatii.st_size," fisier");
            writeUserId(fileDescriptorOut, informatii.st_uid);
            writeTime(fileDescriptorOut, informatii.st_mtime);
            writeHardLinks(fileDescriptorOut,informatii.st_nlink);
            writeUserAccessRights(fileDescriptorOut,informatii.st_mode,"");
            writeGroupAccessRights(fileDescriptorOut,informatii.st_mode,"");
            writeOthersAccessRights(fileDescriptorOut,informatii.st_mode,"");
            if(close(fileDescriptorIn)!=0){
                perror("Nu s-a putut inchide fisierul.\n");
                exit(-1);
            }
            if(close(fileDescriptorOut)!=0){
                perror("Nu s-a putut inchide fisierul.\n");
                exit(-1);
            }
            break;
        }
    case DIRECTOR:{
            int fileDescriptorOut = createFile("statistica.txt");
            writeName(fileDescriptorOut, name,"director");
            writeUserId(fileDescriptorOut, informatii.st_uid);
            writeUserAccessRights(fileDescriptorOut,informatii.st_mode,"");
            writeGroupAccessRights(fileDescriptorOut,informatii.st_mode,"");
            writeOthersAccessRights(fileDescriptorOut,informatii.st_mode,"");
            if(close(fileDescriptorOut)!=0){
                perror("Nu s-a putut inchide fisierul.\n");
                exit(-1);
            }
            break;
    }
    case LINK:{
            int fileDescriptorOut = createFile("statistica.txt");
            writeName(fileDescriptorOut, name,"legatura");
            struct stat informatiiFisierTarget;
            int stat_link = stat(name,&informatiiFisierTarget);
            if(stat_link==-1)
            {
                perror("S-a produs o eroare la aflarea atributelor fisierului.");
                exit(-1);
            }
            writeSize(fileDescriptorOut,informatii.st_size," legatura");
            writeSize(fileDescriptorOut,informatiiFisierTarget.st_size," fisier");
            writeUserAccessRights(fileDescriptorOut,informatii.st_mode," legatura");
            writeGroupAccessRights(fileDescriptorOut,informatii.st_mode," legatura");
            writeOthersAccessRights(fileDescriptorOut,informatii.st_mode," legatura");
            if(close(fileDescriptorOut)!=0){
                perror("Nu s-a putut inchide fisierul.\n");
                exit(-1);
            }
            break;
    }
    default:{
            printf("Nu se poate realiza scrierea in fisier in conditiile date.\n");
            break;
    }
    }
}

DIR *openDirector(char *path){
    DIR *director;
    if((director=opendir(path))==NULL){
        perror("Nu s-a putut deschide directorul.\n");
        exit(-1);
    }
    return director;
}

void getAtributes(char *name){
    int stat_out=lstat(name,&informatii);
    if(stat_out==-1)
    {
        perror("S-a produs o eroare la aflarea atributelor.");
        exit(-1);
    }
}

void readDirector(DIR *director,char *name){
    struct dirent *informatiiDirector;
    while ((informatiiDirector=readdir(director))!=NULL)
    {
        printf("%s\n",informatiiDirector->d_name);
        char path[1000];
        sprintf(path,"%s/%s",name,informatiiDirector->d_name);
        getAtributes(path);
        type_of_file type = typeOfFile(informatii.st_mode);
        writeStatisticsByType(type,informatiiDirector->d_name);
    }
    
}

int main(int argc,char *argv[]){
    testArgs(argc,argv);
    
    DIR *director = openDirector(argv[1]);

    //getAtributes(argv[1]);
    
    //type_of_file type = typeOfFile(informatii.st_mode);
    //writeStatisticsByType(type,argv[1]);
    readDirector(director,argv[1]);
    /*int stat_out=lstat(argv[1],&informatii);
    if(stat_out==-1)
    {
        perror("S-a produs o eroare la aflarea atributelor.");
        exit(-1);
    }*/

    //type_of_file type = typeOfFile(informatii.st_mode);
    return 0;
}