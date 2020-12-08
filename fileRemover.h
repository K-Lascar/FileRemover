#ifndef _FILEREMOVER_GUARD
#define _FILEREMOVER_GUARD

#include <time.h>

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

#endif