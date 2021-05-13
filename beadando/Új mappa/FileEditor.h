#ifndef FILE_EDITOR_INCLUDED
#define FILE_EDITOR_INCLUDED

#include "FileEditor.c"


void addPatient(const char* filename);
void editingExistingPatient(const char* filename);
void deleteExistingPatient(const char* filename);
void readLine();
void makeList(char *filename);

#endif 