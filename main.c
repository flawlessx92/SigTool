/**
*   @file   main.c
*   @brief  Tool for ripping/null-signing Android ROM ZIP images.
*   @version 1.0
*   @author Fardjad Davari
*   @date   06/07/2012
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// constants
#define SUCCESS 0
#define ARG_ERROR 1
#define IO_ERROR 100
#define VERSION "1.0"
#define SIG_LENGTH 0x100

// forward declarations
void printUsage();
char* getFileName(char*);
void rip(char*);
void nullSign(char*);
size_t getFileSize(FILE*);
void showError(int, char*);

// global variables
char* exePath = NULL;

/**
*   Main method
*   @param[in] argc Number of arguments
*   @param[in] argv Arguments vector
*/
int main(int argc, char* argv[]) {
    exePath = argv[0];

    printf("Rom Signature Tool V%s by Fardjad.\n", VERSION);

    if (argc != 3) {
        printUsage();
    }

    char* command = argv[1];
    char* fileName = argv[2];

    if ((stricmp(command, "rip")) == 0) {
        rip(fileName);
    } else if ((stricmp(command, "nullsign") == 0)) {
        nullSign(fileName);
    } else {
        printUsage();
    }

    return SUCCESS;
}

/**
*   Extracts file name from path
*   @param[in] path Path to extract file name from
*   @return File name extracted from given path
*/
char* getFileName(char* path) {
    char* fileName = strrchr(path, '\\');

    if (fileName == NULL) {
        fileName = path;
    } else {
        fileName++;
    }

    return fileName;
}

/**
*   Calculates the given file size
*   @param[in] pFile pointer to file stream
*   @return Given file size
*/
size_t getFileSize(FILE* pFile) {
    fseek(pFile, 0, SEEK_END);
    size_t fileSize = ftell(pFile);
    rewind(pFile);
    return fileSize;
}

/**
*   Removes first 256 byte from the given file.
*   @param[in] fileName Name of file to remove signature from
*/
void rip(char* fileName) {
    FILE* pFile = fopen(fileName, "rb+");
    if (pFile == NULL) {
        showError(errno, "Can't open file.");
    }

    if (getFileSize(pFile) < SIG_LENGTH) {
        showError(IO_ERROR, "Invalid file.");
    }

    printf("Creating backup...\n");

    char* bakFileName = strcat(fileName, ".bak");
    FILE *pBakFile = fopen(bakFileName, "wb+");

    if (pBakFile == NULL) {
        showError(errno, "Can't create backup file.");
    }

    char buffer[BUFSIZ];

    size_t n;
    while((n = fread(buffer, sizeof(char), sizeof(buffer), pFile)) > 0) {
        fwrite(buffer, sizeof(char), n, pBakFile);
    }

    printf("Ripping signature...\n");

    fseek(pBakFile, SIG_LENGTH, SEEK_SET);
    fseek(pFile, 0, SEEK_SET);

    while((n = fread(buffer, sizeof(char), sizeof(buffer), pBakFile)) > 0) {
        fwrite(buffer, sizeof(char), n, pFile);
    }

    printf("Done.\n");

    fclose(pBakFile);
    fclose(pFile);
}

/**
*   Writes 256 null bytes at beginning of the given file
*   @param[in] fileName Name of file to null-sign
*/
void nullSign(char* fileName) {
    FILE* pFile = fopen(fileName, "rb+");
    if (pFile == NULL) {
        showError(errno, "Can't open file.");
    }

    printf("Creating backup...\n");

    char* bakFileName = strcat(fileName, ".bak");
    FILE *pBakFile = fopen(bakFileName, "wb+");

    if (pBakFile == NULL) {
        showError(errno, "Can't create backup file.");
        return;
    }

    char buffer[BUFSIZ];

    size_t n;
    while((n = fread(buffer, sizeof(char), sizeof(buffer), pFile)) > 0) {
        fwrite(buffer, sizeof(char), n, pBakFile);
    }

    printf("Null-signing...\n");

    fseek(pBakFile, 0, SEEK_SET);
    fseek(pFile, 0, SEEK_SET);

    char zero[SIG_LENGTH] = {'\0'};

    fwrite(zero, sizeof(char), sizeof(zero), pFile);

    while((n = fread(buffer, sizeof(char), sizeof(buffer), pBakFile)) > 0) {
        fwrite(buffer, sizeof(char), n, pFile);
    }

    printf("Done.\n");

    fclose(pBakFile);
    fclose(pFile);
}

/**
*   Prints app usage to standard error stream and exits with exit code ARG_ERROR
*/
void printUsage() {
    char* exeName = getFileName(exePath);

    char* format = "Usage: %s rip|nullsign <FILE>\n";
    char* usage = (char*) malloc(strlen(exeName) + strlen(format));
    sprintf(usage, format, exeName);
    fputs(usage, stderr);
    exit(ARG_ERROR);
}

/**
*   Prints the given error message to standard error stream and exits with the given error number
*   @param number       Error number
*   @param description  Error description
*/
void showError(int number, char* description) {
    char* format = "Error %d: %s\n";
    char* error = (char*) malloc((number / 10) + 1 + strlen(description) + strlen(format));
    sprintf(error, format, number, description);
    fputs(error, stderr);
    exit(number);
}
