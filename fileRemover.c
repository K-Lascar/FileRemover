#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

// MAX LENGTHS
#define MAX_PATHNAME_LEN 100000
#define MAX_GREP_TERM_REGEX 5000

// ARGV Indexes
#define ARG_INDEX 1
#define TERM_INDEX 2

// Length of argument type.
#define ARG_TYPE_LEN 3

// Boolean conditions.
#define TRUE 1
#define FALSE 0

// Types of calls.
#define CALL_NORM 0
#define CALL_PREFIX 1
#define CALL_TIME 2
#define CALL_GREP 3

// Directory
#define DIR_CHAR '/'
#define DIR_DELIM "/"
#define CUR_DIR "./"

// File Descriptors
#define READ_END 0
#define WRITE_END 1

static time_t retrieveTime(char path[MAX_PATHNAME_LEN]);

static time_t stripTime(char *datetime);

static int calculateTimeDiff(char *datetime, char filePath[MAX_PATHNAME_LEN]);

static int findPaths(char *searchTerm, char filePath[MAX_PATHNAME_LEN],
                     int callType);
static void removeNameFileDir(char **nameCollection, int numNames, 
                     int isPrefix);

static void removeDirectory(char path[MAX_PATHNAME_LEN]);

static void removeGreppedFile(char **nameCollection, int numNames, int callType);

static int grepFiles(char *grepTerms, char filePath[MAX_PATHNAME_LEN]);

int main (int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "%s <option argument> <term/s>\n", argv[0]);
        exit(1);
    }

    if (strlen(argv[ARG_INDEX]) != 2) {
        fprintf(stderr, "Provide a valid argument\n");
        exit(1);
    }

    char argumentType[ARG_TYPE_LEN];
    strcpy(argumentType, argv[ARG_INDEX]);

    if (!strcmp(argumentType, "-n")) {
        removeNameFileDir(&argv[TERM_INDEX], argc - 2, CALL_NORM);
    } else if (!strcmp(argumentType, "-p")) {
        removeNameFileDir(&argv[TERM_INDEX], argc - 2, CALL_PREFIX);
    } else if (!strcmp(argumentType, "-t")) {
        removeNameFileDir(&argv[TERM_INDEX], argc - 2, CALL_TIME);
    } else if (!strcmp(argumentType, "-g")) {
        removeGreppedFile(&argv[TERM_INDEX], argc - 2, CALL_GREP);
    }

    return 0;
}

// This function will remove a directory using posix_spawnp. This is a better
// alternative than rmdir function. However is unsafe, as removals are permanent.
static void removeDirectory(char path[MAX_PATHNAME_LEN]) {
    
    // Load arguments for posix_spawnp (we're using recursive removal).
    pid_t pid;
    extern char **environ;
    char *rmDirArgv[] = {"rm", "-r", path, NULL};
    if (posix_spawnp(&pid, rmDirArgv[0], NULL, NULL, rmDirArgv, environ) != 0) {
        perror("posix_spawnp");
        exit(1);
    }

    // Wait for child process to die.
    if (waitpid(pid, NULL, 0) == EOF) {
        perror("waitpid");
        exit(1);
    }
}

// Retrieves the last status change time.
static time_t retrieveTime(char path[MAX_PATHNAME_LEN]) {
    struct stat file_stat;
    if (lstat(path, &file_stat) == EOF) {
        perror(path);
        exit(1);
    }
    return file_stat.st_ctime;
}

// This function strips a given Time much like strptime.
static time_t stripTime(char *datetime) {

    // Reference: https://stackoverflow.com/a/24701952
    struct tm timeSplit;
    int numVariables = sscanf(datetime, "%d/%d/%d %d:%d:%d",  &timeSplit.tm_mday,
    &timeSplit.tm_mon, &timeSplit.tm_year, &timeSplit.tm_hour, 
    &timeSplit.tm_min, &timeSplit.tm_sec);

    // If we receive an error, or invalid number of variables.
    if (numVariables == EOF || numVariables != 6) {
        perror(datetime);
        exit(1);
    }

    // tm_year is the time since 1900.
    timeSplit.tm_year -= 1900;

    // tm_mon range from 0-11.
    timeSplit.tm_mon -= 1;

    // Retrieves currentTime.
    time_t currentTime;
    time(&currentTime);
    struct tm *currentTimeStruct = localtime(&currentTime);

    // Check for valid time.
    if (timeSplit.tm_year > (currentTimeStruct->tm_year) || timeSplit.tm_year < 70) {
        fprintf(stderr, "Invalid Year Defined\n");
        exit(1);
    }

    time_t resultTime = mktime(&timeSplit);

    // If the resultTime - currentTime > 0, it means that the time is in the future.
    if (difftime(resultTime, currentTime) > 0) {
        fprintf(stderr, "Time is in the future\n");
        exit(1);
    }
    return resultTime;
}

// In this function we calculate the time difference between the datetime passed,
// and filePath identified.
static int calculateTimeDiff(char *datetime, char filePath[MAX_PATHNAME_LEN]) {

    // %m/%d/%y %H:%M:%S format. strptime(datetime, "%D %T", &timeRetrieved)
    // We cannot use strptime since it's POSIX standard. 
    // Requires a macro to be defined which breaks functionality
    // of other functions.
    time_t timeArg = stripTime(datetime);
    time_t fileTime = retrieveTime(filePath);
    double difference = difftime(fileTime, timeArg);

    // If difference, is less than 0, it means it is older (which is intended).
    int result =  difference < 0.0;
    return result;
}

// This function will find and delete any paths that relate to the searchTerm.
// It will search recursively only if we observe a directory. It will return
// the number of files removed.
static int findPaths(char *searchTerm, char filePath[MAX_PATHNAME_LEN], 
                     int callType) {
    int numFilesRemoved = 0;
    char tempPath[MAX_PATHNAME_LEN];
    strcpy(tempPath, filePath);
    DIR *de = opendir(filePath);
    if (de == NULL) {
        perror(filePath);
        exit(1);
    }

    struct dirent *dirp;
    while ((dirp = readdir(de)) != NULL) {
        if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {

            strcat(tempPath, dirp->d_name);

            // Check if file is directory or regular file.
            if (dirp->d_type == DT_DIR) {

                // Concat directory.
                strcat(tempPath, DIR_DELIM);
                if (callType == CALL_TIME && 
                    calculateTimeDiff(searchTerm, tempPath) || 
                    callType == CALL_PREFIX && 
                    !strncmp(searchTerm, dirp->d_name, strlen(searchTerm))) {

                    // Remove directory and add to numFilesRemoved.
                    removeDirectory(tempPath);
                    numFilesRemoved += 1;
                } else {
                    numFilesRemoved += findPaths(searchTerm, tempPath, callType);
                }
            } else if (dirp->d_type == DT_REG) {
                if (callType == CALL_NORM && !strcmp(dirp->d_name, searchTerm) ||
                    (callType == CALL_PREFIX && !strncmp(searchTerm, 
                    dirp->d_name, strlen(searchTerm))) ||
                    callType == CALL_TIME && 
                    calculateTimeDiff(searchTerm, tempPath) ||
                    callType == CALL_GREP && grepFiles(searchTerm, tempPath)) {
        
                    // If we can remove a directory then we add to the numFilesRemoved.
                    numFilesRemoved += remove(tempPath) == 0 ? 1: 0;
                }
            }

            // Resets tempPath.
            strcpy(tempPath, filePath);
        }
    }
    closedir(de);
    return numFilesRemoved;
}

// This function receives terms and the numTerms. Based on the callType we, 
// will remove the files or directories passed.
static void removeNameFileDir(char **termCollection, int numTerms, int callType) {
    for (int i = 0; i < numTerms; i++) {
        char filePath[MAX_PATHNAME_LEN];
        strcpy(filePath, CUR_DIR);
        int numFilesRemoved = findPaths(termCollection[i], filePath, callType);
        if (numFilesRemoved) {
            printf("File: %-10s had %d instances removed\n", termCollection[i],
            numFilesRemoved);
        } else {
            printf("No file exist for filename: %s\n", termCollection[i]);
        }
    }
}

// This function will remove files based on grep.
static void removeGreppedFile(char **termCollection, int numTerms, int callType) {
    char *grepTerms = calloc(MAX_GREP_TERM_REGEX, sizeof(char));
    strcat(grepTerms, termCollection[0]);
    for (int i = 1; i < numTerms; i++) {

        // This is syntax for grep regex, when you want to have one or more terms.
        strcat(grepTerms, "|");
        strcat(grepTerms, termCollection[i]);
    }

    // filePath.
    char filePath[MAX_PATHNAME_LEN];
    strcpy(filePath, CUR_DIR);

    int numFilesRemoved = findPaths(grepTerms, filePath, callType);
    if (numFilesRemoved) {
        printf("File: %-10s had %d instances removed\n", grepTerms, numFilesRemoved);
    } else {
        printf("No such term/s exist for %s\n", grepTerms);
    }
    free(grepTerms);
}

// This function will grep files using posix_spawnp. It will check if the given file
// contains any of the grepTerms.
static int grepFiles(char *grepTerms, char filePath[MAX_PATHNAME_LEN]) {
    int fileDesc[2];
    if (pipe(fileDesc) == EOF) {
        perror("pipe");
        exit(1);
    }

    posix_spawn_file_actions_t actions;
    if (posix_spawn_file_actions_init(&actions) != 0) {
        perror("posix_spawn_file_actions_init");
        exit(1);
    }

    // Put close to read end of file descriptor.
    if (posix_spawn_file_actions_addclose(&actions, fileDesc[READ_END]) != 0) {
        perror("posix_spawn_file_actions_addclose");
        exit(1);
    }

    // Changing the printing such that, the result doesn't print to stdout (fileno = 1),
    // and stores it in the write end of the file descriptor.
    if (posix_spawn_file_actions_adddup2(&actions, fileDesc[WRITE_END], 1) != 0) {
        perror("posix_spawn_file_actions_adddup2");
        exit(1);
    }

    pid_t pid;
    extern char **environ;
    // Cannot use grep -l grepTerms <*.* - all files or *.c all c files>

    // -E advanced regex. -l meaning list files. -w search by word.
    char *grepArgv[] = {"grep", "-E", "-l", "-w", grepTerms, filePath, NULL};
    if (posix_spawnp(&pid, grepArgv[0], &actions, NULL, grepArgv, environ) != 0) {
        perror("posix_spawnp");
        exit(1);
    }

    close(fileDesc[1]);

    // Open file using read end of fileDescriptor.
    FILE *inputStream = fdopen(fileDesc[READ_END], "r");
    if (inputStream == NULL) {
        perror("fdopen");
        exit(1);
    }

    int letter = 0;
    // If we do get a result from inputStream other than EOF, then it is
    // successful.
    int isSuccessful = ((letter = fgetc(inputStream)) == EOF) ? FALSE: TRUE;
    fclose(inputStream);

    if (waitpid(pid, NULL, 0) == EOF) {
        perror("waitpid");
        exit(1);
    }

    // Destroy actions.
    posix_spawn_file_actions_destroy(&actions);
    return isSuccessful;
}