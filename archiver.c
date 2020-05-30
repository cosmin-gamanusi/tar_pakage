// Cosmin Gamanusi 312CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NR_MAXIM_PARAMETRII 3
#define NR_MAXIM_PARAMETRII_LINIE 9

union record {
  char charptr[512];
  struct header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[8];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
  } header;
};

int octalToDecimal(int n) {
  int numar = n;
  int dec_value = 0;

  int baza = 1;

  int temp = numar;
  while (temp) {
    int ultimaCifra = temp % 10;
    temp = temp / 10;

    dec_value += ultimaCifra * baza;

    baza = baza * 8;
  }

  return dec_value;
}

void cleanNewLine(char *bufferCitire) {
  if (bufferCitire[strlen(bufferCitire) - 1] == '\n') {
    bufferCitire[strlen(bufferCitire) - 1] = '\0';
  }
}

void separareInput(char *bufferCitire, char vectorParametrii[4][1000]) {
  int counter = 0;
  const char s[1] = " ";
  char *token;
  token = strtok(bufferCitire, s);
  // bag fiecare parametru in vectorul de parametrii
  while (token != NULL && counter <= NR_MAXIM_PARAMETRII) {
    strcpy(vectorParametrii[counter], token);
    token = strtok(NULL, s);
    counter++;
  }
}

int verificareInput(char vectorParametrii[4][1000], int *returnIndex) {
  // verific primul parametru pt a afla daca este o comanda valida
  if (strcmp(vectorParametrii[0], "create") != 0 &&
      strcmp(vectorParametrii[0], "list") != 0 &&
      strcmp(vectorParametrii[0], "extract") != 0) {
    return -1;
  } else {
    // verific daca comanda "create" e completa
    if (strcmp(vectorParametrii[0], "create") == 0 &&
        strcmp(vectorParametrii[1], "") != 0 &&
        strcmp(vectorParametrii[2], "") != 0) {
      *returnIndex = 1;
      return 1;
    }
    // verific daca comanda "list" e completa
    if (strcmp(vectorParametrii[0], "list") == 0 &&
        strcmp(vectorParametrii[1], "") != 0) {
      *returnIndex = 2;
      return 1;
    }
    // verific daca comanda "extract" e completa
    if (strcmp(vectorParametrii[0], "extract") == 0 &&
        strcmp(vectorParametrii[1], "") != 0 &&
        strcmp(vectorParametrii[2], "") != 0) {
      *returnIndex = 3;
      return 1;
    }
    return -1;
  }
}

void breakLine(char *fullRow, char vectorBreakLine[9][100], char *s) {
  int counter = 0;
  char *token;
  token = strtok(fullRow, s);
  while (token != NULL && counter <= NR_MAXIM_PARAMETRII_LINIE) {
    strcpy(vectorBreakLine[counter], token);
    token = strtok(NULL, s);
    counter++;
  }
}

int rightsCounterIntFunction(char vectorBreakLine[9][100],
                             int rightsCounterInt) {
  rightsCounterInt = 0;
  int aux;
  for (int i = 0; i < 3; i++) {
    aux = 0;
    if (vectorBreakLine[0][3 * i + 1] == 'r') {
      aux += 4;
    }
    if (vectorBreakLine[0][3 * i + 2] == 'w') {
      aux += 2;
    }
    if (vectorBreakLine[0][3 * i + 3] == 'x')
      aux += 1;
    rightsCounterInt = rightsCounterInt * 10 + aux;
  }
  return rightsCounterInt;
}

int makeTime(struct tm timp, int year, int mon, int mday, int hour, int min,
             int sec) {
  timp.tm_year = year - 1900;
  timp.tm_mon = mon - 1;
  timp.tm_mday = mday;
  timp.tm_hour = hour;
  timp.tm_min = min;
  timp.tm_sec = sec;
  timp.tm_isdst = -2;
  return mktime(&timp);
}

int mtime(char *date, char *hour, char vectorBreakLine[9][100],
          struct tm timp) {
  breakLine(date, vectorBreakLine, "-");
  // conversie din char in int
  int year = atoi(vectorBreakLine[0]);
  int month = atoi(vectorBreakLine[1]);
  int day = atoi(vectorBreakLine[2]);

  breakLine(hour, vectorBreakLine, ":");

  int ora = atoi(vectorBreakLine[0]);
  int minutes = atoi(vectorBreakLine[1]);
  int secconds = atoi(vectorBreakLine[2]);
  return makeTime(timp, year, month, day, ora, minutes, secconds);
}

int convertDecimalToOctal(int decimalNumber) {
  int octalNumber = 0, i = 1;
  while (decimalNumber != 0) {
    octalNumber += (decimalNumber % 8) * i;
    decimalNumber /= 8;
    i *= 10;
  }
  return octalNumber;
}

void conversieStringToOctal(char *string) {
  // conversie din char in int
  int x = atoi(string);
  // conversie din int in octal
  int y = convertDecimalToOctal(x);
  // conversie din int in char
  sprintf(string, "%d", y);
}

int openedVerify(FILE *f) {
  if (f == NULL) {
    return 0;
  }
  return 1;
}

void resetString(char *string) {
  for (unsigned int i = 0; i < strlen(string); i++) {
    string[i] = '\0';
  }
}
int findNum(int n, int x) {
  int r = (n + x) % x;

  if (r == 0)
    return n;
  else
    return n + x - r;
}

int create(char vectorParametrii[4][1000], char vectorBreakLine[9][100]) {
  union record row;
  char fullRow[512], rightsCounterChar[10];
  int rightsCounterInt;
  struct tm timp;

  memset(&row, 0, 512);
  memset(row.header.mode, '0', 7);
  memset(row.header.uid, '0', 7);
  memset(row.header.gid, '0', 7);
  memset(row.header.size, '0', 11);
  memset(row.header.devmajor, '\0', 7);
  memset(row.header.devminor, '\0', 7);
  memset(row.header.mtime, '0', 11);

  // deschid fisierul files.txt
  FILE *f_ReadFiles = fopen("files.txt", "r");
  if (openedVerify(f_ReadFiles) == 0) {return 0;}
  // deschid fisierul usermap.txt
  FILE *f_ReadUser = fopen("usermap.txt", "r");
  if (openedVerify(f_ReadUser) == 0) {return 0;}
  // deschid fisierul de scriere
  FILE *f_write = fopen(vectorParametrii[1], "wb");
  // citesc fiecare linie din files.txt, o sparg cu strtok si o bag fiecare
  // cuvant intr un vector de stringuri
  char parametrul_2[100], c; int counter = 0;
  while (fgets(fullRow, 512, f_ReadFiles)) {
    counter++; cleanNewLine(fullRow);
    breakLine(fullRow, vectorBreakLine, " ");
    // citesc in structura ce am nevoie de pe fiecare linie
    strcpy(row.header.name, vectorBreakLine[8]);
    rightsCounterInt =
        rightsCounterIntFunction(vectorBreakLine, rightsCounterInt);
    // conversie din int in char
    sprintf(rightsCounterChar, "%d", rightsCounterInt);
    strcpy(row.header.mode + strlen(row.header.mode) -
            strlen(rightsCounterChar), rightsCounterChar);
    // conversie string to int
    int size = atoi(vectorBreakLine[4]);
    int sizeOctal = convertDecimalToOctal(size);
    // conversie int to octal
    sprintf(row.header.size + 5, "%d", sizeOctal);
    row.header.typeflag = '0';
    strcpy(row.header.magic, "GNUtar ");
    strcpy(row.header.uname, vectorBreakLine[2]);
    strcpy(row.header.gname, vectorBreakLine[3]);
    int mTimeInt =
        mtime(vectorBreakLine[5], vectorBreakLine[6], vectorBreakLine, timp);
    // conversie din int in octal
    sprintf(row.header.mtime, "%06o", mTimeInt);
     strcpy(parametrul_2, vectorParametrii[2]);
    strcat(parametrul_2, vectorBreakLine[8]);
    // deschid fisierul din nume
    FILE *f_fisier = fopen(parametrul_2, "rb");
    if (openedVerify(f_fisier) == 0) {return 0;}
    // citesc si din usermap.txt
    while (fgets(fullRow, 512, f_ReadUser)) {
      breakLine(fullRow, vectorBreakLine, ":");
      for (int i = 0; i < 7; i++) {
        if (strcmp(vectorBreakLine[i], row.header.uname) == 0) {
          conversieStringToOctal(vectorBreakLine[2]);
          strcpy(row.header.uid + strlen(row.header.uid) -
                  strlen(vectorBreakLine[2]), vectorBreakLine[2]);
          conversieStringToOctal(vectorBreakLine[3]);
          strcpy(row.header.gid + strlen(row.header.gid) -
                  strlen(vectorBreakLine[3]), vectorBreakLine[3]);
        }
      }
    }
    memset(row.header.chksum, ' ', 8);
    unsigned int chksumInt = 0;
    for (int i = 0; i < 512; i++) {
      chksumInt += row.charptr[i];
    }
    sprintf(row.header.chksum, "%06o", chksumInt);
    row.header.chksum[6] = 0;
    fwrite(&row, 1, sizeof(row), f_write);
    long sizeInt = octalToDecimal(sizeOctal);
    int restBlocksNumber = sizeInt % 512;
    while (1 == 1) {
      fread(&c, sizeof(char), 1, f_fisier);
      if (feof(f_fisier))break;
      fwrite(&c, 1, sizeof(c), f_write);
    }

    if (restBlocksNumber != 0) {
      memset(fullRow, 0, 512 - restBlocksNumber);
      fwrite(fullRow, 1, 512 - restBlocksNumber, f_write);
    }
    fclose(f_fisier);
  }
  fclose(f_write);
  fclose(f_ReadUser);
  fclose(f_ReadFiles);
  return 1;
}

int list(char vectorParametrii[4][1000]) {
  union record row;
  FILE *f_fisier = fopen(vectorParametrii[1], "rb");
  if (openedVerify(f_fisier) == 0) {
    return 0;
  }
  char buffer[1000];
  while (1 == 1) {
    fread(buffer, 1, 100, f_fisier);
    if (feof(f_fisier)) {
      break;
    }
    strcpy(row.header.name, buffer);
    resetString(buffer);
    fseek(f_fisier, 24, SEEK_CUR);

    fread(buffer, 1, 12, f_fisier);
    strcpy(row.header.size, buffer);
    resetString(buffer);
    int sizeOctal = atoi(row.header.size);
    int sizeDec = octalToDecimal(sizeOctal);
    int jump = findNum(sizeDec, 512);

    fseek(f_fisier, 412 - 36 + jump, SEEK_CUR);

    printf(">%s\n", row.header.name);
  }
  fclose(f_fisier);
  return 1;
}

int extract(char vectorParametrii[4][1000]) {
  union record row;
  FILE *f_fisier = fopen(vectorParametrii[2], "rb");
  if (openedVerify(f_fisier) == 0) {
    return 0;
  }
  char fisier[100] = {"extracted_"};
  strcat(fisier, vectorParametrii[1]);
  FILE *f_write = fopen(fisier, "wb");
  if (openedVerify(f_write) == 0) {
    return 0;
  }
  char buffer[1000];
  while (1 == 1) {
    fread(buffer, 1, 100, f_fisier);
    if (feof(f_fisier)) {
      break;
    }
    strcpy(row.header.name, buffer);
    resetString(buffer);
    fseek(f_fisier, 24, SEEK_CUR);

    fread(buffer, 1, 12, f_fisier);
    strcpy(row.header.size, buffer);
    resetString(buffer);
    int sizeOctal = atoi(row.header.size);
    int sizeDec = octalToDecimal(sizeOctal);
    int jump = findNum(sizeDec, 512);
    int rest = sizeDec % 512;

    if (strcmp(row.header.name, vectorParametrii[1]) != 0) {
      fseek(f_fisier, 412 - 36 + jump, SEEK_CUR);
    } else {
      fseek(f_fisier, 412 - 36, SEEK_CUR);
      int seturi = sizeDec / 512;
      for (int i = 0; i < seturi; i++) {
        fread(buffer, 1, 512, f_fisier);
        fwrite(buffer, 1, 512, f_write);
      }
      char c;
      if (rest != 0) {
        for (int i = 0; i < rest; i++) {
          fread(&c, sizeof(char), 1, f_fisier);
          fwrite(&c, 1, sizeof(c), f_write);
        }
      }
      fclose(f_write);
      fclose(f_fisier);
      return 1;
    }
  }
  return 1;
}

int main() {
  char bufferCitire[100], vectorParametrii[4][1000] = {""},
                          vectorBreakLine[9][100];
  int returnIndex;
  fgets(bufferCitire, 100, stdin);
  cleanNewLine(bufferCitire);

  while (strcmp(bufferCitire, "exit") != 0) {
    separareInput(bufferCitire, vectorParametrii);
    if (verificareInput(vectorParametrii, &returnIndex) == -1) {
      printf("Wrong command!\n");
    } else {
      if (returnIndex == 1) {
        if (create(vectorParametrii, vectorBreakLine) == 0) {
          printf(">Eroare\n");
        } else if (create(vectorParametrii, vectorBreakLine) == 1) {
          printf(">Done\n");
        }
      } else if (returnIndex == 2) {
        // resetez vectoruBreakLine
        for (int i = 0; i < 9; i++) {
          strcpy(vectorBreakLine[i], "\0");
        }
        if (list(vectorParametrii) == 0) {
          printf(">File not found!\n");
        }
      } else if (returnIndex == 3) {
        // resetez vectoruBreakLine
        for (int i = 0; i < 9; i++) {
          strcpy(vectorBreakLine[i], "\0");
        }
        if (extract(vectorParametrii) == 0) {
          printf(">File not found!\n");
        } else {
          printf(">File extracted!\n");
        }
      }
    }
    // resetez vectorul de parametrii
    for (int i = 0; i < 3; i++) {
      strcpy(vectorParametrii[i], "");
    }
    // citesc o noua comanda
    fgets(bufferCitire, 100, stdin);
    cleanNewLine(bufferCitire);
  }
}
