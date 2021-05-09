#include "FileEditor.h"

void menu(){
    printf("\n");
    printf("<------------------------------------>\n");
    printf("Válasszon az elábbi lehetőségekből!\n");
    printf("1 - Adatfelvétel\n");
    printf("2 - Adatmódosítás\n");
    printf("3 - Adattörlés\n");
    printf("4 - Oltásra várók listája\n");
    printf("5 - Kilépés\n");
    printf("<------------------------------------>\n");
    printf("\n");
}

int main(void){
    char *filename = "storage.bin";
    int c;
    menu();
    scanf("%i", &c);
    getchar();
    while(c != 5 ){
        switch(c){
            case 1 : addPatient(filename); break;
            case 2 : editingExistingPatient(filename); break;
            case 3 : deleteExistingPatient(filename); break;
            case 4 : makeList(filename); break;
            default : printf("Nem megfelelő menüpont\n");
        }  
        menu();
        scanf("%i", &c);
        getchar();
    } 
}

