#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int hossz(char * string);
bool egyezik(char *str1, char * str2);
char * levag(char * str, int number);
void karaktercsere(char *str, char a, char b);
int szovegkeres(char *str, char *a);
void ertekadas(char * str1, char * str2);

int main()
{
    char string[] = "HelloWorld";
    char string2[] = "HelloWorld2";
    printf("String: %s\n",string);

    printf("Length: %d\n", hossz(string));
    printf("Egyezik: %d\n", egyezik(string,string2));
    printf("Levág: %s\n", levag(string, 3));
    karaktercsere(string,  'l',  's');
    printf("Karaktercsere: %s\n", string);
    printf("szovegkereses %d \n",szovegkeres("Hajra Fradi", "ra"));
    ertekadas(string, "fradi");
    printf("szovegkereses %d \n",string);
    return 0;
}


//1. Készítsen hossz függvényt, mely egy paraméterként átadott szöveg
//karaktereinek számát adja vissza! A megoldás során ne használja a string.h
//függvényeit! Optimalizálja a megoldást!
//pl. x = hossz(str);
int hossz(char * string){
    int i=0;
    while (*string != 0){
        ++string;
        ++i;
    }
    return i;
}
//2. Készítsen egyezik függvényt, mely két paraméterként átadott szöveg
//egyezésekor igaz, ellenkező esetben hamis értéket ad vissza! A megoldás
//során ne használja a string.h függvényeit! Optimalizálja a megoldást!
//pl. if (egyezik(str1, str2)) …
// if (egyezik(str, "Hajra Fradi!")) …
bool egyezik(char *str1, char * str2){
    bool l = true;
    while(l && *str1 != 0){
        if(*str2 == 0 || *str1 != *str2){
            l = false;
        }
        ++str1;
        ++str2;
    }
    return l && (*str2 == 0) ;
}
//3. Készítsen levag függvényt, mely paraméterei egy szöveg és egy szám (db).
//A függvény a szöveg elejéről távolítson el db számú karaktert! A megoldás
//során ne használja a string.h függvényeit! Optimalizálja a megoldást!
//pl. levag(str,6);
char * levag(char *str, int number){
    return str+number;
}

//4. Készítsen karaktercsere függvényt, mely paraméterei egy szöveg és két
//karakter (c1, c2). A függvény cserélje ki a szöveg összes c1 karakterét c2-
//re! A megoldás során ne használja a string.h függvényeit! Optimalizálja a
//megoldást!
//pl. karaktercsere(str, 'a', '#');

void karaktercsere(char *str, char a, char b){
    while( *str != 0 ){
        if( *str == a){
            *str = b;
        }
        ++str;
    }
}

//5. Készítsen szovegkeres függvényt, mely paraméterei két szöveg, és
//eredménye a második szöveg első előfordulásának helye az első szövegben,
//vagy -1 ha az első szöveg nem tartalmazza másodikat! A megoldás során ne
//használja a string.h függvényeit! Optimalizálja a megoldást!
//pl. pos = szovegkeres(str1, str2);
// pos = szovegkeres("Hajra Fradi", "ra");
int szovegkeres(char *str, char *a){
    int number = 0;
    char * start ;
    start = a;
    while( *str != 0 ){
        while( *a != 0 && *str == *a ){
            ++a;
            ++str;
        }
        if(*a==0){
            number +=1;
        }
        a = start;
        ++str;
    }
    return number;
}

//6. Készítsen ertekadas függvényt, mely az első paraméterként átadott
//karaktertömbnek értékül adja a második paraméterként átadott szöveget! A
//megoldás során ne használja a string.h függvényeit! Optimalizálja a
//megoldást!
//pl. ertekadas(str1, str2);
// ertekadas(str1, "Hajra Fradi");

void ertekadas(char * str1, char * str2){
    while( *str2 != 0 ){
        printf("%c %c \n", *str1, *str2);
        *str1 = *str2;
        ++str1;
        ++str2;
    }
    printf("%s", str1);
}
//7. Hozza létre dinamikusan a 9 karakter hosszú szöveg tárolására alkalmas
//str stringet, majd adja neki értékül az "123456789" karakterláncot
//(használja a 6. feladatban létrehozott ertekadas függvényt), és írja ki a
//konzolra str értékét! Ezt követően módosítsa str méretét 80 byte-ra, majd
//ismét írja ki az értékét a konzolra! Végül módosítsa str méretét
//100’000’000’000 byte-ra, majd harmadszor is írja ki az értékét a konzolra!
//Ha bármelyik méretmódosítás során hiba következik be, azt saját
//hibaüzenettel jelezze! A megoldás során ne használja a string.h
//függvényeit!