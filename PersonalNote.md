This project inspired by what I learned from COMP1521. It was meant to act as a challenge, and to have a deeper understandings to content taught.

One special component of the course was this function called POSIX_SPAWN, which during the time I didn't necessarily grapple with the implementation for this. I felt because of my vague understanding of this function, I thought it was necessarily to use in a project!

## Challenges:

Certain functionalities, I was unable to complete as it would either break other components or was not allowed for example, I would've liked the convenience of 

```c	
// This is from man pages.
#define _XOPEN_SOURCE // This macro broke other functions.
#include <time.h>

char *strptime(const char *s, const char *format, struct tm *tm);
```

This macro as described 

[here]: https://stackoverflow.com/a/5724485	"here"

adds extra functionality that are only defined in X/Open POSIX standards, doing this by changing how libraries behave as described 

[here]: https://stackoverflow.com/a/5379283	"here"

Even though this convenience, would've slightly improved efficiency, it gave a deeper understanding on what is involved in this function, which is a benefit nonetheless.



There was another hurdle, that was hindered for some unknown reason, when you load an argument for grep as the following:

```c
char *grepTerms = "Hello|Goodbye"
char *grepArgv[] = {"grep", "-E", "-l", "-w", grepTerms, "*.c", NULL};
if (posix_spawnp(&pid, grepArgv[0], &actions, NULL, grepArgv, environ) != 0) {
    perror("posix_spawnp");
    exit(1);
}
```

This would not work as grep mistook "*.c" as a file, I'm guessing that this functionality is only available on the command line. Where it would list out all the files containing "Hello" or "Goodbye" as a list of files. What I was thinking what we could do is process each of these files individually and remove them using our function ```findPaths```. However in the end, this allowed me to redesign the code so that any file containing these words would be removed if they exist relative to the current directory.

Insightful:

Since we use time_t in a program, which is a typedef of long it is susceptible to the Year 2038 problem. https://en.wikipedia.org/wiki/Year_2038_problem

Which is also used in ```sys/stat.h``` library. Hopefully it is solved!



Thanks for reading and happy coding!