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

#define BUS_SIGNAL SIGUSR1
#define DATA_SIGNAL SIGUSR1
#define HARCRA_FEL SIGUSR1

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

void updateVakcinatedPatient(const char *filename, int searchPatientID) {
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

    while (fcntl(fd, F_SETLK, &fl) == -1) {
        fl.l_type = F_WRLCK; //írásra vagy olvasásra
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        fl.l_pid = getpid();
    }

    while (read(fd, &patient, sizeof(patient)) == sizeof(patient)) {
        if (patient.id == searchPatientID) {
            patient.vakcinated = 1;
            printf("--------%s -nek beadtuk a vakcinát!----------\n", patient.name);
            lseek(fd, (sizeof(struct Patient) * (patient.id - 1)), SEEK_SET);
            if (write(fd, &patient, (sizeof(patient))) < 0) {
                perror("Error updating\n");
                exit(EXIT_FAILURE);
            }
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
struct Patient getPatient(const char *filename, int searchPatientID) {
    struct Patient patient;
    int result = 0;

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

    while (fcntl(fd, F_SETLK, &fl) == -1) {
        fl.l_type = F_RDLCK; //írásra vagy olvasásra
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        fl.l_pid = getpid();
    }

    while (!result && read(fd, &patient, sizeof(patient)) == sizeof(patient) ) {
   //     printf("Keresendő ID-k %i %i\n", patient.id, searchPatientID);
        if (patient.id == searchPatientID) {
            //        printf("Megtaláltam %s\n", patient.name);
            result=1;
        }
    }


    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return patient;
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
               patient.id, patient.name, patient.birthYear, patient.phoneNumber, patient.extra == 1 ? "igen" : "nem",
               patient.vakcinated == 0 ? "nem" : "igen");
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
        if (patient.vakcinated == 0)
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

void handler(int signo) {

}

void bus(int *pipe,  int line, char * filename) {
    printf("ELindult %i. Busz \n", line);
    union sigval category;
    category.sival_int = 1;

 //   printf("kuldi a signalt\n");
    sigqueue(getppid(), HARCRA_FEL, category);
 //   printf("elküldte\n");


    close(pipe[1]);
    int meret;
    //maszkolás
 //   printf("Várok a szülőre %i\n",getpid());

    sigset_t mask3;
    sigfillset(&mask3);
    sigprocmask(SIG_SETMASK, &mask3, NULL);

    signal(DATA_SIGNAL, handler);
    sigdelset(&mask3, DATA_SIGNAL);
    sigsuspend(&mask3);


  //  printf("szülő adott választ %i\n", getpid());

    read(pipe[0], &meret, sizeof(meret));
 //   printf("meret %i busz: %i\n", meret, line);

    int number;
    sleep(1);
 //   printf("elkezdem beolvasni %i\n", line);
    for(int i = 0; i< meret; ++i){
        read(pipe[0], &number, sizeof(number));
 //      printf("ezt olvastam be %i\n",number);

        struct Patient patient = getPatient(filename, number);

        printf("Páciens: %s\n", patient.name);

        printf("Várjuk a %s nevű beteget az %i. busznál", patient.name, line);
        srand(time(NULL));
        int eljott = rand() % 10 + 1;
        printf("Random szám: %i \n", eljott);

        if (eljott <= 9) {
            updateVakcinatedPatient(filename, patient.id);
        }
        sleep(1);
    }
    close(pipe[0]);

    printf("Végeztem %i busz", line);
    exit(0);
}

void startProcess(char *filename) {

    //maszkolás
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    //pipeok
    int pipe1[2];
    pipe(pipe1);
    int pipe2[2];
    pipe(pipe2);


    //adatok beolvasásához előkészületek
    int patientsNumber;
    if ((patientsNumber = getPatientNumber(filename)) > 10) {
        patientsNumber = 10;
    }
    printf("%i", patientsNumber);
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

            // printf("Olvasom %s %i\n", patients[i].name,i);
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

   // printf("Szülő %i \n", getpid());
    //párhuzamosítás
    pid_t bus1 = fork();
    if (bus1) {//szulo

   //    printf("Szülő %i \n", getpid());
        //signal beállítása
        signal(HARCRA_FEL, handler);
        sigemptyset(&mask);
        sigaddset(&mask, HARCRA_FEL);

        siginfo_t info2;
        siginfo_t info;
        struct timespec varakozas;
        varakozas.tv_sec = LONG_MAX;
        varakozas.tv_nsec = 0;
        //várakozás a signálra
    //    printf("Várok a gyerekre!\n");
        sigtimedwait(&mask, &info, &varakozas);
    //    printf("Megkaptam az 1. signalt\n");

        if (patientsNumber > 9) {
            int number = 5;
            pid_t bus2 = fork();
            if (bus2) {
            //szülő
       //        printf("Várok a 2. gyerekre %i \n", getpid());
                sigemptyset(&mask);
                sigaddset(&mask, HARCRA_FEL);
                sigtimedwait(&mask, &info2, &varakozas);
        //        printf("Meegvan a 2. ggyerek1 %i \n", getpid());

                close(pipe1[0]);//olvasás bezárása
                close(pipe2[0]);//olvasás bezárása

                write(pipe1[1], &number, sizeof(number)); //csőbe írás , méret  

                for(i = 0; i<5; ++i){
        //            printf("Átküldtem a patient id-t %i\n", patients[i].id);
                    write(pipe1[1], &patients[i].id, sizeof(patients[i].id)); //csőbe írás , méret  
                }
                
        //        printf("Küldöm az adatokat bus1-nek! %i\n", getpid());
                kill(bus1, DATA_SIGNAL);

                
                write(pipe2[1], &number, sizeof(number)); //csőbe írás , méret  

                while(i<10){
        //            printf("Átküldtem a patient id-t %i\n", patients[i].id);
                    write(pipe2[1], &patients[i].id, sizeof(patients[i].id)); //csőbe írás , méret  
                    ++i;
                }

        //        printf("Küldöm az adatokat bus2-nek! %i\n", getpid());
                kill(bus2, DATA_SIGNAL);

                close(pipe1[1]);
                close(pipe2[1]);

                wait(NULL);
                wait(NULL);
            
            } else {
                //2. gyerek
           //     printf("gyerek1 %i \n", getpid());
                bus(pipe2, 2, filename);
            }
        }else{
            close(pipe1[0]);//olvasás bezárása

            write(pipe1[1], &number, sizeof(number)); //csőbe írás , méret  

            for(i = 0; i<5; ++i){
        //        printf("Átküldtem a patient id-t %i\n", patients[i].id);
                write(pipe1[1], &patients[i].id, sizeof(patients[i].id)); //csőbe írás , méret  
            }
                
        //    printf("Küldöm az adatokat bus1-nek! %i\n", getpid());
            kill(bus1, DATA_SIGNAL);
            close(pipe1[1]);
            wait(NULL);
        }
    } else {//gyerek
    //    printf("gyerek");
        bus(pipe1,  1, filename);
    }

}