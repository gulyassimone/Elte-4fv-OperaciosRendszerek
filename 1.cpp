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
        printf(" (%i) \n név: \t %s \n születési év: \t %i \n telefonszám: \t %s  \n Hajlandó extra díjat fizetni? \t %s \n \n",
               patient.id, patient.name, patient.birthYear, patient.phoneNumber, patient.extra == 1 ? "igen" : "nem");
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
    printf("category");
    psignal(signo, "Handler");
    printf("HARCRA FEL! %i\n", signo);
}

void bus(int *pipe) {
    union sigval category;
    category.sival_int = 1;
    sigqueue(getppid(), BUS1_SIGNAL, category);

    close(pipe[1]);
    int meret;
    read(pipe[0], &meret, sizeof(meret));
    Patient patients[meret];
    read(pipe[0], &patients, meret);
    for (int i = 0; i < meret; ++i) {
        printf("Várjuk a %s nevű beteget", patient.name);
        printf("%i\n", rand() % 10 + 1);
        if (rand() % 10 + 1 <= 9) {
            patients[i].vakcinated = 1;
        }
    }
    close(pipe[0]);
    category.sival_int = 2;
    sigqueue(getppid(), BUS1_SIGNAL, category);
    exit(0);
}

void startProcess(char *filename) {
    //maszkolás
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    //pipe megnyitása
    int pipefd[2];
    pipe(pipefd);

    //adatok beolvasásához előkészületek
    struct Patient patient;
    struct Patient patients[10];
    struct Patient *p = patients;


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
    while (read(fd, &patient, sizeof(patient)) && i < sizeof(patients)) {
        if (patient.vakcinated == 0) {
            printf(" %i \n", patient.vakcinated);
            *p = patient;
            ++p;
            ++i;
        }
    }

    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("File unlock failed!");
        exit(EXIT_FAILURE);
    }

    close(fd);

    if (i < 4) {
        printf("Nincs elég oltásra váró, nem indítok buszt");
        return;
    }
    //párhuzamosítás
    pid_t bus1 = fork();
    if (bus1) {//szulo
        //signal beállítása
        signal(BUS1_SIGNAL, handler);
        sigemptyset(&mask);
        sigaddset(&mask, BUS1_SIGNAL);
        siginfo_t info;
        struct timespec varakozas;
        varakozas.tv_sec = LONG_MAX;
        varakozas.tv_nsec = 0;

        //várakozás a signálra
        sigtimedwait(&mask, &info, &varakozas);
        if (info.si_value.sival_int == 1)
            printf("Harcra fel!");
        close(pipefd[0]);//olvasás bezárása
        write(pipefd[1], &i, sizeof(i)); //csőbe írás , méret
        write(pipefd[1], &patients, i); //csőbe írás

        for (int i = 0; i < meret; ++i) {
            printf("Várjuk a %s nevű beteget", patient.name);
            printf("%i\n", rand() % 10 + 1);
        }
        printf("Tisztelt %s, várjuk oltásra az 1.-es busznál", patient.name); //üzenet
        close(pipefd[1]);
    } else {//gyerek
        bus(pipefd);
    }
    if (i > 9) {
        // bus2=fork();
    }

}
