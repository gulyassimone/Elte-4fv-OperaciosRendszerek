/*Egy taxitársaságánál a fuvarozásszervezés segítségére alkalmazást készítünl. A központ(szülő folyamat) folyamatosan várja egy utas jelentését.
Mikor beérkezik egy utas(gyerek folyamat) kérése, a központ hozzárendel egy kiszolgáló autót(másik gyerek folyamat). A szülő folyamat mindig várja
meg a gyerek folyamatok végét.
A, Az utas jelentkezésekor csövön bemondja a lakcímét, ezt a központ kiolvassa, majd képernyőre írja. A lakcímet agumentumként adjuk meg.
B, Központ a taxisnak elkülfi üzenetsoron a lakcímet és az utas telefonszámát, amit a PID reprezentál. Taxis kiolvassa, képernyőre írja, majd nyugtázásképpen visszaírja üzenetsoron, hogy elindult az utasért.
C, Taxis, amint a címre érkezik, betelefonál az utasnak(jelzést küld).. Az utas bemondja a címet, amit egy távolság érték (random egész szám 1 és 20 között), amit jelzéssel hoz a taxis tudomására. Taxis ezt kiolvassa és válaszul a képernyőre írja az árat.
(Ár = Távolság*300+700)
D, A taxis ezt a bevételt osztott memóriában lejelenti a központ felé. Védje a műveletet szemaforral.
*/

#include <stdio.h>      //printf
#include <string.h>     //strlen
#include <unistd.h>     //fork
#include <sys/wait.h>   //wait
#include <signal.h>     //maszkolás
#include <sys/msg.h>    //uzenetkuldes
#include <limits.h>     //LONG_MAX
#include <time.h>       //time  
#include <stdlib.h>     //srand/rand
#include <sys/ipc.h>    //semget
#include  <sys/shm.h>   //shmget
#include <sys/sem.h> //shmget

#define UTAS_JELENTKEZIK SIGUSR1

//4
#define TAXIS_JELENTKEZIK SIGUSR2
#define UTAS_TAVOLSAG SIGUSR2

struct uzenet_pid_t
{
    long kategoria;
    pid_t pid;
};

void handler(int signum)
{
    //Ne az alapértelmezett handler fusson le
}

int main(int argc, char **argv) //a lakcímet agumetumként adjuk meg
{
    if(argc < 2)
    {
        printf("Kerlek adj meg egy cimet!\n");
        return 1;
    }
    //szignálmaszk beállítása
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);
    //névtelen cső létrehozása
    int cso[2];
    pipe(cso);

    pid_t utas, taxi;
    utas = fork();
    if(utas)
    {//szülő
        close(cso[1]); //szülő nem ír a csőbe
        //várakozás utasra
        signal(UTAS_JELENTKEZIK, handler);
        sigdelset(&mask, UTAS_JELENTKEZIK);
        sigsuspend(&mask);
        //olvasas a csobol(meret, szoveg);
        int meret;
        read(cso[0], &meret, sizeof(meret));
        char cim[meret];
        read(cso[0],cim,meret);
        printf("Kozpont-cim %s\n",cim);
        close(cso[0]);
        //taxi indit
        //3 "elindult" ferjen bele az uzenetbe
        if(meret<9)
            meret = 9;
        //3 uzenetsor letrehozasa
        int uzenetsor = msgget(IPC_PRIVATE,0600 | IPC_CREAT);
        struct 
        {
            long kategoria;
            char szoveg[meret];
        }uzenet_szoveg;
        struct uzenet_pid_t uzenet_pid;

        //5 osztott memória létrehozása
        int osztmem = shmget(IPC_PRIVATE, sizeof(int), 0600 | IPC_CREAT);
        int *ar = shmat(osztmem, NULL,0);
        //5 szemaforkészlet létrehozása
        int szemafor = semget(IPC_PRIVATE, 1 ,0600 | IPC_CREAT);
        semctl(szemafor, 0, SETVAL, 0); //nincs kiolvasható adat
        struct sembuf muvelet[1];

        taxi = fork();
        if(taxi)
        {//szulo
            //3 uzenet taxisnak cim 1-es kategoria
            uzenet_szoveg.kategoria = 1;
            strcpy(uzenet_szoveg.szoveg, argv[1]);
            msgsnd(uzenetsor, &uzenet_szoveg, sizeof(uzenet_szoveg.szoveg),0);
            //3 uzenet taxisnak telefonszam 2-es kategoria
            uzenet_pid.kategoria = 2;
            uzenet_pid.pid = utas;
            msgsnd(uzenetsor, &uzenet_pid, sizeof(uzenet_pid.pid),0);
            //3 taxis uzenetenek kiolvasasa(3-as kategoria)
            msgrcv(uzenetsor, &uzenet_szoveg, meret, 3, 0);
            printf("Kozpont - taxis uzenete: %s \n", uzenet_szoveg.szoveg);
            //5 Szemafor csökkentése, ha van olvasható adat
            
            muvelet[0].sem_num = 0;
            muvelet[0].sem_flg = 0;
            muvelet[0].sem_op = -1;
                
            semop(szemafor, muvelet, 1);
            //5 Ár kiiratása osztott memóriából
            printf("Központ - ár %i\n", *ar);
            //szulo varja meg a gyerekfolyamatok veget

            wait(NULL);
            wait(NULL);

        }else{
            //taxi
                //2-esert nem kell megcsinalni
                //3 kiolvassa a cimet az uzenetsorbol(1-es kategoria)
                msgrcv(uzenetsor, &uzenet_szoveg, meret, 1, 0);
                printf("taxis - cim: %s\n", uzenet_szoveg.szoveg);
                //3 kiolvassa a telefonszamot(pid, 2-es kategoria)
                msgrcv(uzenetsor, &uzenet_pid, sizeof(uzenet_pid.pid),2,0);
                printf("Taxis - telefonszam: %i\n", uzenet_pid.pid);
                //3 visszakuldi hogz elindult(3-as kategoria)
                uzenet_szoveg.kategoria = 3;
                strcpy(uzenet_szoveg.szoveg, "Elindult");
                msgsnd(uzenetsor, &uzenet_szoveg, sizeof(uzenet_szoveg.szoveg),0);

                //4 telefonál az utasnak 
                kill(uzenet_pid.pid, TAXIS_JELENTKEZIK);
                //4 cim fogadása
                signal(UTAS_TAVOLSAG, handler);
                sigemptyset(&mask);
                sigaddset(&mask, UTAS_TAVOLSAG);
                siginfo_t info;
                struct timespec varakozas;
                varakozas.tv_sec = LONG_MAX;
                varakozas.tv_nsec = 0;
                sigtimedwait(&mask, &info,&varakozas);
                //4 ár kiszámítása
                printf("Taxis - Ár : %i * 300+700=%i Ft \n", 
                        info.si_value.sival_int, info.si_value.sival_int*300+700);
                //5 Összeg beírása osztott memóriába
                *ar = info.si_value.sival_int*300+700; //ar értéke
                //5 szemafor felemelése
                muvelet[0].sem_num = 0;
                muvelet[0].sem_flg = 0;
                muvelet[0].sem_op = 1;
                semop(szemafor, muvelet, 1);

        }
    }else{
        //utas
        close(cso[0]); //a gyerek nem olvas a csobe
        int meret = strlen(argv[1])+1;//szoveg + '\0'
        //meres es cim csobe irasa
        write(cso[1], &meret, sizeof(meret));
        write(cso[1], argv[1], meret);
        //signal kuldese
        kill(getppid(), UTAS_JELENTKEZIK);
        close(cso[1]);
        //4 várja taxis telefonhívását(kell a pid a válaszküldéshez)
        signal(TAXIS_JELENTKEZIK, handler);
        sigemptyset(&mask);
        sigaddset(&mask, TAXIS_JELENTKEZIK);
        siginfo_t info;
        struct timespec varakozas;
        varakozas.tv_sec = LONG_MAX;
        varakozas.tv_nsec = 0;
        sigtimedwait(&mask, &info,&varakozas);
        //t cim sorsolása és küldése signállal taxisnak
        srand(time(NULL));
        union sigval cel;
        cel.sival_int =  rand() % 20 + 1;
        sigqueue(info.si_pid, UTAS_TAVOLSAG, cel);
        printf("UTAS - CEL: %i\n", cel.sival_int);
    }
    return 0;
}