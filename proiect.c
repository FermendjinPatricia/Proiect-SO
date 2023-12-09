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
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>

void testArgs(int argc, char *argv[]){
    if(argc != 4){                          // testeaza numarul argumentelor
        printf("Usage %s\n",argv[1]);
        exit(-1);
    }
    if(isalpha(argv[3][0])==0){             // testeaza daca caracterul dat este unul alfanumeric
        printf("Usage %s\n",argv[0]);
        exit(-1);
    }
}

// Functii pentru deschidere/creare fisier

int openFile(char *name){
    int fin=open(name, O_RDWR, S_IRWXU);
    if(fin==-1){
        printf("Nu s-a putut deschide fisierul cu numele %s\n",name);
        exit(-1);
    }
    return fin;
}

int createFile(char *name, char *path){
    char pathBuffer[100];
    sprintf(pathBuffer,"%s/%s",path,name);
    int fout=open(pathBuffer, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    if(fout==-1){
        perror("Nu s-a putut crea fisierul.\n");
        exit(-1);
    }
    return fout;
}

// Functie pentru verificare fisier .bmp

int checkBMPFile(char *name){
    const char *ext = strrchr(name, '.');
    if (ext == NULL || strcmp(ext, ".bmp") != 0){
        return 0;
    }
    return 1;
}

// Functii de scriere in fisierul de statistica si de aflare a anumitor informatii despre fisier

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
    lseek(fileDescriptorIn,18, SEEK_SET);
    if(read(fileDescriptorIn, &width, 4)==-1){
        perror("Nu s-a realizat corect citirea inaltimii.\n");
        exit(-1);
    }
    lseek(fileDescriptorIn,0,SEEK_SET);
    return width;
}

int getFilesHeight(int fileDescriptorIn){
    int height;
    lseek(fileDescriptorIn,22, SEEK_SET);
    if(read(fileDescriptorIn,&height,4)==-1){
        perror("Nu s-a realizat corect citirea lungimii.\n");
        exit(-1);
    }
    lseek(fileDescriptorIn,0,SEEK_SET);
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

void writeNewLine(int fd) {
    if(write(fd, "\n", 1) == -1) {
        perror("Nu s-a putut scrie newline.\n");
        exit(-1);
    }
}

typedef enum type_of_file{
    REGULAR,DIRECTOR,LINK
} type_of_file;

type_of_file typeOfFile(mode_t mode){
    if(S_ISREG(mode)!=0) return REGULAR; //regular file
    if(S_ISDIR(mode)!=0) return DIRECTOR; // director
    if(S_ISLNK(mode)!=0) return LINK; //link
    
    return -1;
    
}

// Functia de scriere in statistica.txt

struct stat informatiiFisier;

void getAtributes(char *name){
    int stat_out=lstat(name,&informatiiFisier);
    if(stat_out==-1)
    {
        printf("S-a produs o eroare la aflarea atributelor fisierului:%s.\n",name);
        exit(-1);
    }
}

int writeStatisticsByType(type_of_file type, char *pathIn, char *name, char *pathOut){
    int numberOfLines=0;
    switch (type)
    {
    case REGULAR:{
            char nameOfFile[100];
            sprintf(nameOfFile,"%s_%s",name,"statistica.txt");
            int fileDescriptorOut = createFile(nameOfFile,pathOut);
            int fileDescriptorIn = openFile(pathIn);
            writeNewLine(fileDescriptorOut);
            writeName(fileDescriptorOut, name,"fisier");
            writeSize(fileDescriptorOut,  informatiiFisier.st_size," fisier");
            writeUserId(fileDescriptorOut, informatiiFisier.st_uid);
            writeTime(fileDescriptorOut, informatiiFisier.st_mtime);
            writeHardLinks(fileDescriptorOut,informatiiFisier.st_nlink);
            writeUserAccessRights(fileDescriptorOut,informatiiFisier.st_mode,"");
            writeGroupAccessRights(fileDescriptorOut,informatiiFisier.st_mode,"");
            writeOthersAccessRights(fileDescriptorOut,informatiiFisier.st_mode,"");
            if(close(fileDescriptorIn)!=0){
                perror("Nu s-a putut inchide fisierul.\n");
                exit(-1);
            }
            if(close(fileDescriptorOut)!=0){
                perror("Nu s-a putut inchide fisierul.\n");
                exit(-1);
            }
            numberOfLines+=9;
            break;
        }
    case DIRECTOR:{
            char nameOfFile[100];
            sprintf(nameOfFile,"%s_%s",name,"statistica.txt");
            int fileDescriptorOut = createFile(nameOfFile,pathOut);
            writeNewLine(fileDescriptorOut);
            writeName(fileDescriptorOut, name,"director");
            writeUserId(fileDescriptorOut, informatiiFisier.st_uid);
            writeUserAccessRights(fileDescriptorOut,informatiiFisier.st_mode,"");
            writeGroupAccessRights(fileDescriptorOut,informatiiFisier.st_mode,"");
            writeOthersAccessRights(fileDescriptorOut,informatiiFisier.st_mode,"");
            if(close(fileDescriptorOut)!=0){
                perror("Nu s-a putut inchide fisierul.\n");
                exit(-1);
            }
            numberOfLines+=6;
            break;
    }
    case LINK:{
            char nameOfFile[100];
            sprintf(nameOfFile,"%s_%s",name,"statistica.txt");
            int fileDescriptorOut = createFile(nameOfFile,pathOut);
            writeNewLine(fileDescriptorOut);
            writeName(fileDescriptorOut, name,"legatura");
            struct stat informatiiFisierTarget;
            int stat_link = stat(pathIn,&informatiiFisierTarget);
            if(stat_link==-1)
            {
                perror("S-a produs o eroare la aflarea atributelor fisierului.\n");
                exit(-1);
            }
            writeSize(fileDescriptorOut,informatiiFisier.st_size," legatura");
            writeSize(fileDescriptorOut,informatiiFisierTarget.st_size," fisier");
            writeUserAccessRights(fileDescriptorOut,informatiiFisier.st_mode," legatura");
            writeGroupAccessRights(fileDescriptorOut,informatiiFisier.st_mode," legatura");
            writeOthersAccessRights(fileDescriptorOut,informatiiFisier.st_mode," legatura");
            if(close(fileDescriptorOut)!=0){
                perror("Nu s-a putut inchide fisierul.\n");
                exit(-1);
            }
            numberOfLines+=7;
            break;
    }
    default:{
            printf("Nu se poate realiza scrierea in fisier in conditiile date.\n");
            break;
    }
    }
    return numberOfLines;
}

// Functia de deschidere a directorului

DIR *openDirector(char *path){
    DIR *director;
    if((director=opendir(path))==NULL){
        perror("Nu s-a putut deschide directorul.\n");
        exit(-1);
    }
    return director;
}

// Functii pentru convertirea imaginii in tonuri de gri

int getFilesBitCount(int fileDescriptorIn){
    int bitCount;
    lseek(fileDescriptorIn,28,SEEK_SET);
    if(read(fileDescriptorIn,&bitCount,2)==-1){
        perror("Nu s-a realizat corect citirea bitCount-ului.\n");
        exit(-1);
    }
    lseek(fileDescriptorIn,0,SEEK_SET);
    return bitCount;
}

int getRedPixel(int fileDescriptorIn){
    int red;
    if((read(fileDescriptorIn,&red,1))==-1){
        perror("Nu s-a putut citi pixelul rosu.\n");
        exit(-1);
    }
    return red;
}

int getGreenPixel(int fileDescriptorIn){
    int green;
    if((read(fileDescriptorIn,&green,1))==-1){
        perror("Nu s-a putut citi pixelul verde.\n");
        exit(-1);
    }
    return green;
}

int getBluePixel(int fileDescriptorIn){
    int blue;
    if((read(fileDescriptorIn,&blue,1))==-1){
        perror("Nu s-a putut citi pixelul albastru.\n");
        exit(-1);
    }
    return blue;
}

void rewritePixels(int fileDescriptorIn, int width, int height){
    lseek(fileDescriptorIn,54,SEEK_SET);
    for(int i=0;i<width;i++){
        for(int j=0;j<height;j++){
            int redPixel = getRedPixel(fileDescriptorIn);
            int greenPixel = getGreenPixel(fileDescriptorIn);
            int bluePixel = getBluePixel(fileDescriptorIn);
            int greyPixel = 0.299*redPixel + 0.587*greenPixel + 0.114*bluePixel;
            lseek(fileDescriptorIn,-3,SEEK_CUR);
            write(fileDescriptorIn,&greyPixel,1);
            write(fileDescriptorIn,&greyPixel,1);
            write(fileDescriptorIn,&greyPixel,1);
        }
    }
    lseek(fileDescriptorIn,0,SEEK_SET);

    
}

void closePipeReadEnd(int pipeFileDescriptor[]){
    if(close(pipeFileDescriptor[0])==-1){               // Inchiderea capatului de citire
        perror("Nu s-a putut inchide capatul de citire al pipe-ului.\n");
         exit(-1);
    }
}

void closePipeWriteEnd(int pipeFileDescriptor[]){
    if(close(pipeFileDescriptor[1])==-1){               // Inchiderea capatului de scriere
        perror("Nu s-a putut inchide capatul de scriere al pipe-ului.\n");
         exit(-1);
    }
}

// Functia de parcurgere a directorului de intrare

void readDirector(DIR *director,char *name, char *pathOut, char *argument){
    struct dirent *informatiiDirector;
    while ((informatiiDirector=readdir(director))!=NULL)
    {
        pid_t pid;
        int status, cod;
        if(checkBMPFile(informatiiDirector->d_name)==1){        // Daca in directorul parcurs exista un fisier .bmp
            if((pid=fork())<0){                                 // Se creeaza un proces fiu
                perror("Eroare proces BMP.\n");
                exit(1);
            }
            if(pid==0){                                         // Interiorul procesului fiu care realizeaza convertirea imaginii .bmp in tonuri de gri
                char path[1000];
                sprintf(path,"%s/%s",name,informatiiDirector->d_name);
                //printf("Full path %s\n", path);
                int fileDescriptorIn = openFile(path);
                if(getFilesBitCount(fileDescriptorIn)!=24){
                    perror("Nu se poate converti aceasta imagine.\n");
                    if(close(fileDescriptorIn)==-1){
                    perror("Nu s-a putut inchide fisierul.\n");
                    exit(-1);
                    }
                    exit(-1);
                }
                int height = getFilesHeight(fileDescriptorIn);
                int width = getFilesWidth(fileDescriptorIn);
                rewritePixels(fileDescriptorIn,width,height);
                if(close(fileDescriptorIn)==-1){
                    perror("Nu s-a putut inchide fisierul.\n");
                    exit(-1);
                }
                exit(0);
            }// incheiere procesului fiu care se ocupa de fisierele cu extensia .bmp
            pid=wait(&status);
            cod = WEXITSTATUS(status);
            printf("S-a incheiat procesul cu pid-ul %d si codul %d.\n",pid,cod);
        }
        else{
            printf("Numele fisierului din director:     %s.\n",informatiiDirector->d_name);
            int pipeFileDescriptor[2];
            if(pipe(pipeFileDescriptor)<0)                          // Crearea pipe-ului
	        {
	            perror("Eroare la crearea pipe-ului\n");
	            exit(1);
	        }

            if((pid=fork())<0){                                     // Crearea primului proces fiu pentru fisierele obisnuite 
                perror("Eroare");
                exit(1);
            }
            if(pid==0){                                             // Interiorul procesului fiu care se ocupa de scrierea fisierelor de statistica
                closePipeReadEnd(pipeFileDescriptor);
                
                char path[1000];
                sprintf(path,"%s/%s",name,informatiiDirector->d_name);
                getAtributes(path);
                type_of_file type = typeOfFile(informatiiFisier.st_mode);
                int numberOfLinesWritten = writeStatisticsByType(type,path,informatiiDirector->d_name,pathOut);


                if(typeOfFile(informatiiFisier.st_mode)==REGULAR){
                    if((dup2(pipeFileDescriptor[1],1)) == -1){          // Redirectarea iesirii standard a programului la capatul de scriere al pipe-ului
                        perror("Nu s-a putut redirecta iesirea standard la primul pipe.\n");
                        exit(-1);
                    }                      
                    if (execlp("cat", "cat", path, NULL) == -1) {       // generarea continutului fisierului, transmis prin pipe catre al doilea proces
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }
                    
                }
                else{
                    printf("Nu se poate extrage continutul fisierului.\n");
                    exit(numberOfLinesWritten);
                }
                // Daca folosesc exec, codul dupa el nu se mai ruleaza, doar in caz de eroare.
                //closePipeWriteEnd(pipeFileDescriptor);
                //exit(numberOfLinesWritten); 
            }// incheierea procesului fiu
        // Proces parinte care primeste numarul de linii scrise in fiecare dintre procesele fiu de mai sus
            pid=wait(&status);
            cod = WEXITSTATUS(status);
            printf("S-a incheiat procesul cu pid-ul %d si codul %d.\n",pid,cod);
            
            closePipeWriteEnd(pipeFileDescriptor);
            
            /*int fd = fileno(stdout); // Obține descriptorul de fișier pentru stdout

            if (isatty(fd)) {
                printf("Ieșirea standard nu este redirecționată către un pipe.\n");
            } else {
                printf("Ieșirea standard este redirecționată către un pipe sau alt descriptor de fișier.\n");
            }*/

            int pipeFileDescriptorPropozitii[2];
            int contorPropozitii=0;
            if(pipe(pipeFileDescriptorPropozitii)<0)                          // Crearea pipe-ului
	        {
	            perror("Eroare la crearea pipe-ului\n");
	            exit(1);
	        }

            if ((pid=fork())<0){                                    // Crearea celui de al doilea proces fiu pentru fisierele obisnuite
                perror("Eroare");
                exit(1);
            }
            if(pid==0){                                             // Interiorul celui de-al doilea proces fiu care se ocupa de numararea prop corecte
                closePipeReadEnd(pipeFileDescriptorPropozitii);

                                     
                if((dup2(pipeFileDescriptor[0],0)) == -1){                                // Redirectarea intrarii standard la capatul de citire al pipe-ului
                    perror("Nu s-a putut redirecta intrarea standard la primul pipe\n");
                    exit(-1);
                }

                if((dup2(pipeFileDescriptorPropozitii[1],1)) == -1){        // Redirectarea iesirii standard a programului la capatul de scriere al pipe-ului
                    perror("Nu s-a putut redirecta iesirea standard la al doilea pipe\n");
                    exit(-1);
                } 

                if (execlp("bash", "bash", "script.sh", argument, NULL) == -1) {       // generarea continutului fisierului, transmis prin pipe catre al doilea proces
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }// incheiere proces fiu
            pid=wait(&status);
            cod = WEXITSTATUS(status);
            printf("S-a incheiat procesul cu pid-ul %d si codul %d.\n",pid,cod);

            closePipeWriteEnd(pipeFileDescriptorPropozitii);
            closePipeReadEnd(pipeFileDescriptor);


            int propozitii;
            if((dup2(pipeFileDescriptorPropozitii[0],0)) == -1){
                perror("Nu s-a putut redirecta intrarea standard la al doilea pipe\n");
                exit(-1);
            }
            
            int result = scanf("%d",&propozitii);

            printf("%d",propozitii);
            contorPropozitii+=propozitii;
            
            closePipeReadEnd(pipeFileDescriptorPropozitii);
            printf("Au fost identificate in total %d propozitii corecte care contin caracterul %s.\n",contorPropozitii,argument);
        }
    }
    
}


int main(int argc, char *argv[]){
    testArgs(argc,argv);
    DIR *director = openDirector(argv[1]);

    readDirector(director,argv[1],argv[2],argv[3]);
    return 0;
}