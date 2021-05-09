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
#define UTAS_JELENTKEZIK SIGUSR1

void handler(int signum)
{
    //Ne az alapértelmezett handler fusson le
}

int main(int argc, char **argv) //a lakcímet agumetumként adjuk meg
{
    if(argc < 2)
    {
        printf("Kerlek adj meg egy cimet");
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
        taxi = fork();
        if(taxi)
        {//szulo
        //a szulo varja meg a gyerekfolyamatok veget
            wait(NULL);
            wait(NULL);

        }else{
            //taxi
                //2-esert nem kell megcsinalni

        }
    }else{
        close(cso[0]); //a gyerek nem olvas a csobe
        int meret = strlen(argv[1])+1;//szoveg +\0
        //meres es cim csobe irasa
        write(cso[1], &meret, sizeof(meret));
        write(cso[1], argv[1], meret);
        //signal kuldese
        kill(getppid(), UTAS_JELENTKEZIK);
        close(cso[1]);
    }
    return 0;
}