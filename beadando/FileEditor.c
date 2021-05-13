//
// Created by gulyas on 2021. 04. 03..
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "FileEditor.h"
#include <limits.h>     //LONG_MAX
#include <time.h>       //time  
#include <signal.h>     //maszkolás
#include <sys/msg.h>    //uzenetkuldes
#include <limits.h>     //LONG_MAX
#include <time.h>       //time  
#include <stdlib.h>     //srand/rand

#define BUS1_SIGNAL SIGUSR1
#define BUS2_SIGNAL SIGUSR2
#define DATA_SIGNAL SIGUSR1

#define MIN_PATIENT 4
#define MAX_PATIENT 10
#define MAX 100


struct Patient {
    char name[MAX];
    int birthYear;
    char phoneNumber[MAX];
    int extra;
    int vakcinated;
    int id;
};

void readName(char *patientName) {
    char name[MAX];
    fgets(name, sizeof(name), stdin);
    while (*name == '\n') {
        fgets(name, sizeof(name), stdin);
    }
    name[strcspn(name, "\n")] = 0;
    memcpy(patientName, name, strlen(name) + 1);
}


void addPatient(const char *filename) {
    struct Patient patient, patient2;
    int id = 1;

//O_CREAT - ha nem létezik a fájl, akkor létrehozza
//O_EXCL önmagában nem használható, csak az O_CREAT-el együtt használható, mindenképpen hozd létre a fájlt, ha létezik akkor hibával tér vissza
//O_TRUNC rossz szándékű, függetlenül attól, hogy create van vagy nincs akkor is megsemmísiti a fájlt
    struct flock fl;
    fl.l_type = F_WRLCK; //írásra vagy olvasásra
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();

    int fd;
    fd = open(filename, O_RDWR | O_APPEND | O_CREAT, 0777);
    if (fd == -1) {
        perror("File open failed!");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File lock failed!");
        exit(EXIT_FAILURE);
    }


    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        id = id + 1;
    }


    printf("Gépelje be a nevet! \n");
    readName(patient.name);
    printf("Gépelje be a születési évet! \n");
    scanf("%i", &patient.birthYear);
    printf("Adja meg a telefonszámát! \n");
    scanf("%s", &patient.phoneNumber);
    printf("Hajlandó extra díjat fizetni? (1=igen, 0= nem) \n");
    scanf("%i", &patient.extra);
    patient.id = id;
    patient.vakcinated = 0;

    write(fd, &patient, sizeof(patient));

    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd);

    printf("Az oltásra jelentkező sikeresen hozzáadva.\n");
}


void editingExistingPatient(const char *filename) {
    struct Patient patient;
    char patientName[MAX];
    int patientBirthYear;
    int result = 0;
    printf("Kérem a nevet!\n");
    readName(patientName);
    printf("Gépelje be a születési évet! \n");
    scanf("%i", &patientBirthYear);

    struct flock fl;
    fl.l_type = F_WRLCK; //írásra vagy olvasásra
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();

    int fd;

    fd = open(filename, O_RDWR);

    if (fd == -1) {
        perror("File open failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File lock failed!");
        exit(EXIT_FAILURE);
    }

    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        if (strcmp(patient.name, patientName) == 0 && patientBirthYear == patient.birthYear) {
            printf(" (%i) \n név: \t %s \n születési év: \t %i \n telefonszám: \t %s  \n Hajlandó extra díjat fizetni? \t %s \n \n",
                   patient.id, patient.name, patient.birthYear, patient.phoneNumber,
                   patient.extra == 1 ? "igen" : "nem");
            printf("Adja meg az új telefonszámát! \n");
            scanf("%s", &patient.phoneNumber);
            printf("Hajlandó extra díjat fizetni? (1=igen, 0= nem) \n");
            scanf("%i", &patient.extra);

            printf("Oltva van? (1=igen, 0= nem) \n");
            scanf("%i", &patient.vakcinated);

            lseek(fd, (sizeof(struct Patient) * (patient.id - 1)), SEEK_SET);
            if (write(fd, &patient, (sizeof(patient))) < 0) {
                perror("Error updating\n");
                exit(EXIT_FAILURE);
            }
            result = 1;
        }
    }

    if (result == 0) {
        printf("\n Nincs ilyen adatokkal eltárolt személy az adatbázisban! \n");
    }

    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void updateVakcinatedPatient(const char *filename, struct Patient * patients) {
    struct Patient patient;

    struct flock fl;
    fl.l_type = F_WRLCK; //írásra vagy olvasásra
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();

    int fd;

    fd = open(filename, O_RDWR);

    if (fd == -1) {
        perror("File open failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File lock failed!");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        if (strcmp(patient.name, patients[i].name) == 0 && patient.birthYear == patients[i].birthYear) {
            patient.vakcinated = patients[i].vakcinated;
            lseek(fd, (sizeof(struct Patient) * (patient.id - 1)), SEEK_SET);
            if (write(fd, &patient, (sizeof(patient))) < 0) {
                perror("Error updating\n");
                exit(EXIT_FAILURE);
            }
            ++i;
        }
    }


    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd);
}
void clearVakcinatedPatient(const char *filename) {
    struct Patient patient;

    struct flock fl;
    fl.l_type = F_WRLCK; //írásra vagy olvasásra
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();

    int fd;

    fd = open(filename, O_RDWR);

    if (fd == -1) {
        perror("File open failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File lock failed!");
        exit(EXIT_FAILURE);
    }

    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        patient.vakcinated = 0;
        printf("Visszaállítottam %s %i\n", patient.name, patient.vakcinated);
        lseek(fd, (sizeof(struct Patient) * (patient.id - 1)), SEEK_SET);
        if (write(fd, &patient, (sizeof(patient))) < 0) {
            perror("Error updating\n");
            exit(EXIT_FAILURE);
        }
    }


    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void deleteExistingPatient(const char *filename) {
    struct Patient patient;
    char patientName[MAX];
    int patientBirthYear, fd, fd_tmp;
    int result = 0;
    printf("Kérem a nevet!\n");
    readName(patientName);
    printf("Gépelje be a születési évet! \n");
    scanf("%i", &patientBirthYear);

    struct flock fl;
    fl.l_type = F_WRLCK; //írásra vagy olvasásra
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();


    fd = open(filename, O_RDWR);

    if (fd == -1) {
        perror("File open failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File lock failed!");
        exit(EXIT_FAILURE);
    }
    struct flock fl_tmp;
    fl_tmp.l_type = F_WRLCK; //írásra vagy olvasásra
    fl_tmp.l_whence = SEEK_SET;
    fl_tmp.l_start = 0;
    fl_tmp.l_len = 0;
    fl_tmp.l_pid = getpid();

    fd_tmp = open("tmp.bin", O_RDWR | O_APPEND | O_CREAT, 0777);

    if (fd_tmp == -1) {
        perror("TMP file open failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd_tmp, F_SETLK, &fl_tmp) == -1) {
        perror("Tmp file lock failed!");
        exit(EXIT_FAILURE);
    }

    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        if (strcmp(patient.name, patientName) == 0 && patientBirthYear == patient.birthYear) {
            printf("%s törölve lett az adatbázisból", patientName);
            result = 1;
        } else {
            if (result == 1) {
                --patient.id;
            }
            write(fd_tmp, &patient, sizeof(patient));
        }
    }

    if (result == 0) {
        printf("\n Nincs ilyen adatokkal eltárolt személy az adatbázisban! \n");
    }

    fl_tmp.l_type = F_UNLCK;
    if (fcntl(fd_tmp, F_SETLK, &fl_tmp) == -1) {
        perror("Temp file unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd_tmp);
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd);

    remove(filename);
    rename("tmp.bin", filename);
}

void makeList(char *filename) {
    struct Patient patient;
    char patientName[MAX];
    int patientBirthYear;

    struct flock fl;
    fl.l_type = F_RDLCK; //írásra vagy olvasásra
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();

    int fd;

    fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("File open failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File lock failed!");
        exit(EXIT_FAILURE);
    }

    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        printf(" (%i) \n név: \t %s \n születési év: \t %i \n telefonszám: \t %s  \n Hajlandó extra díjat fizetni? \t %s \n Oltva van? %s \n \n",
               patient.id, patient.name, patient.birthYear, patient.phoneNumber, patient.extra == 1 ? "igen" : "nem", patient.vakcinated == 0 ? "nem":"igen");
    }

    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

int getPatientNumber(char *filename) {
    struct Patient patient;
    int number = 0;

    struct flock fl;
    fl.l_type = F_RDLCK; //írásra vagy olvasásra
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();

    int fd;

    fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("File open failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File lock failed!");
        exit(EXIT_FAILURE);
    }

    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        if(patient.vakcinated == 0)
            ++number;
    }

    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return number;
}

void handler(int signo) 
{

}

void writeToPipe(char * pipe, struct Patient * patients, int size)
{
    int fd=open(pipe,O_WRONLY);
    for(int i = 0; i < size; ++i){
        printf("\nÍrás közben: %s\n", patients[i].name);
        write(fd, &patients[i], sizeof(patients[i]));       
    }
    close(fd);
}
void readFromPipe(char * pipe, struct Patient * patients)
{
    int fd=open(pipe,O_RDONLY);
    struct Patient patient;
    int i = 0;
    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        
        printf("\nOlvasás közben: %s\n", patients[i].name);
        patients[i] = patient;
        ++i;
    }
    close(fd);
}
int createPipe(char * pipe, int piece){
    sprintf(pipe,"/tmp/%d%d",getpid(), piece);
    int fid=mkfifo(pipe, S_IRUSR|S_IWUSR ); 
    if (fid==-1)
    {
	    printf("Error number: %i",errno);
	    exit(EXIT_FAILURE);
    }
    return fid;
}

void bus(int *pipe,  char *pipefd, int line) {
   printf("ELindult %i %i %i" , line, getpid(), getppid());
    union sigval category;
    category.sival_int = 1;
    printf("kuldi a signalt\n");
    if(line==1){
        sigqueue(getppid(), BUS1_SIGNAL, category);
          printf("elküldte\n");
    }else{
        sigqueue(getppid(), BUS2_SIGNAL, category);
          printf("elküldte\n");
    }
  

    close(pipe[1]);
    int meret;
    //maszkolás
    printf("Várok a szülőre\n");
    sigset_t mask3;
    sigfillset(&mask3);
    sigprocmask(SIG_SETMASK, &mask3, NULL);
    signal(DATA_SIGNAL, handler);
    sigdelset(&mask3, DATA_SIGNAL);
    sigsuspend(&mask3);

    
printf("szülő adott választ\n");
    read(pipe[0], &meret, sizeof(meret));
    printf("meret %i busz: %i\n", meret, line);
    close(pipe[0]);
    struct Patient patients[meret];


    printf("elkezdem beolvasni %i", line);


    sigsuspend(&mask3);
    readFromPipe(pipefd, patients);
    printf("beolvastam");
    for(int i = 0; i < meret; ++i)
        printf("Páciens: %s\n", patients[i].name);


    for (int i = 0; i < meret; ++i) {
        printf("Várjuk a %s nevű beteget az %i. busznál",patients[i].name, line);
        int eljott = rand() % 10 + 1;
        if (eljott <= 9) {
            patients[i].vakcinated = 1;
        }
    }
    category.sival_int = 2;
    if(line){
        sigqueue(getppid(), BUS1_SIGNAL, category);
    }else{
        sigqueue(getppid(), BUS2_SIGNAL, category);
    }
    printf("vege");
    writeToPipe(pipefd, patients, meret);
    exit(0);
}

void startProcess(char *filename) {

    //maszkolás
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    sigset_t mask2;
    sigfillset(&mask2);
    sigprocmask(SIG_SETMASK, &mask2, NULL);
    //pipeok 
    int pipe1[2];
    pipe(pipe1);
    int pipe2[2];
    pipe(pipe2);
    char pipefd1[20];
    char pipefd2[20];

    int fid1 = createPipe(pipefd1,1);
    //int fid2 = createPipe(pipefd2,2);

    //adatok beolvasásához előkészületek
    int patientsNumber;
    if((patientsNumber = getPatientNumber(filename))>10){
        patientsNumber = 10;
    }
    printf("%i",patientsNumber);
    struct Patient patients[patientsNumber];
    struct Patient patient;

    struct flock fl;
    fl.l_type = F_RDLCK; //írásra vagy olvasásra
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();

    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("File open failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File lock failed!");
        exit(EXIT_FAILURE);
    }

    int number = getPatientNumber(filename);


    //adatok beolvasása
    int i = 0;
    while (read(fd, &patient, sizeof(patient)) && i < patientsNumber) {
        if (patient.vakcinated == 0) {
            patients[i] = patient;
            
            printf("Olvasom %s %i\n", patients[i].name,i);
            ++i;
        }
    }
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }

    close(fd);

    if (patientsNumber < 4) {
        printf("Nincs elég oltásra váró, nem indítok buszt");
        return;
    }

    printf("ELindult %i " , getpid());
    //párhuzamosítás
    pid_t bus1 = fork();
    if (bus1) {//szulo
        //signal beállítása
        signal(BUS1_SIGNAL, handler);
        sigemptyset(&mask);
        sigaddset(&mask, BUS1_SIGNAL);

        siginfo_t info2;
        siginfo_t info;
        struct timespec varakozas;
        varakozas.tv_sec = LONG_MAX;
        varakozas.tv_nsec = 0;
        //várakozás a signálra
        sigtimedwait(&mask, &info, &varakozas);
        printf("Megkaptam az 1. signalt");
       if (patientsNumber > 9) {
                struct Patient part1[5] ={patients[0],patients[1],patients[2],patients[3],patients[4]};
                struct Patient part2[5] = {patients[5],patients[6],patients[7],patients[8],patients[9]};


                signal(BUS2_SIGNAL, handler);
                sigemptyset(&mask2);    
                sigaddset(&mask2, BUS2_SIGNAL);
                number=5;
                pid_t bus2 = fork();
                if (bus1 > 0){
                    sigtimedwait(&mask2, &info2, &varakozas);
                    if (info.si_value.sival_int == 1 && (info2.si_value.sival_int == 1 || patientsNumber < 10))
                    {
                        printf("Harcra fel!");
                    }
                    printf("Megkaptam a signalt!");
                    close(pipe1[0]);//olvasás bezárása
                    kill(bus1, DATA_SIGNAL);
                    write(pipe1[1], &number, sizeof(number)); //csőbe írás , méret

                    close(pipe2[0]);//olvasás bezárása
                    kill(bus2, DATA_SIGNAL);
                    write(pipe2[1], &number, sizeof(number)); //csőbe írás , méret


                    union sigval category;
                    category.sival_int = 1;

                    writeToPipe(pipefd1, part1, 5);
                    kill(bus1, DATA_SIGNAL);
                    writeToPipe(pipefd2, part2, 5);
                     kill(bus2, DATA_SIGNAL);

                    sigtimedwait(&mask, &info, &varakozas);
                    if (info.si_value.sival_int == 2 && (info2.si_value.sival_int == 2 || patientsNumber < 10))
                    printf("Feldolgozom az adatokat");


                    readFromPipe(pipefd1, part1);
                    updateVakcinatedPatient(filename, part1);
                    
                    readFromPipe(pipefd2, part2);
                    updateVakcinatedPatient(filename, part2);
                    //csőbe írás  
                    wait(NULL);
                    wait(NULL);
                } else {
                    //2. gyerek
                    bus(pipe2, pipefd2, 2); 
                }
        }else{
                if (info.si_value.sival_int == 1 && (info2.si_value.sival_int == 1 || patientsNumber < 10))
                    printf("Harcra fel!");

                close(pipe1[0]);//olvasás bezárása
                printf("Írok a csőbe");
                write(pipe1[1], &patientsNumber, sizeof(patientsNumber)); //csőbe írás , méret
                printf("Csőben van a cucc");
                writeToPipe(pipefd1, patients, patientsNumber);

                sigtimedwait(&mask, &info, &varakozas);
                if (info.si_value.sival_int == 2 && (info2.si_value.sival_int == 2 || patientsNumber < 10))
                    printf("Feldolgozom az adatokat");

                readFromPipe(pipefd1, patients);
                updateVakcinatedPatient(filename, patients);
                //csőbe írás  
                wait(NULL);
                wait(NULL);
        }      

    } else {//gyerek
    printf("gyerek");
        bus(pipe1, pipefd1, 1);   
    }

    unlink(pipefd1);
    unlink(pipefd2);

}
