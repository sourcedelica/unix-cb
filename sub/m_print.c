#include "../menu/menu.h"
#include <string.h>

void *privprint(privs, header, showall)
long    privs;
char *header;
int showall;
{
    int j=0;
    FILE    *pfp;
    char    fname[256], str[256], *formfname();

    strcpy(fname, formfname("privs", "cf") );

    if ((pfp = fopen(fname,"r")) == NULL)
        return(NULL);

    while (fgets(str,256,pfp) != NULL) {
        j = atoi(strtok(str,"\t"));

        if (showall)
            printf("%s%d) %s: %s\n",header,j,strtok(NULL, "\n"),
                (privs & (1 << j)) ? "Yes" : "No");

        else if (privs & (1 << j) )
            printf("%s%s\n",header,strtok(NULL, "\n") );
    }
    fclose(pfp);
}

void plevel(level, header)
int level;
char *header;
{
    FILE    *lfp;
    char    str[256];
    int i;
    char    *strtok(), fname[256];

    strcpy(fname, formfname("levels", "cf") );

    if ((lfp = fopen(fname,"r")) == NULL)
        return;

    while (fgets(str,256,lfp) != NULL) {
        i = atoi(strtok(str,"\t"));

        if (i > level)
            break;

        if (i <= level)
            printf("%s%s\n",header,strtok(NULL,"\n") );
    }
    fclose(lfp);
}
