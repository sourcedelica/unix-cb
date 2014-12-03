/*  Skynet external file transfer protocol handling

    Definitions
*/

#define PF_BATCH    'B'
#define PF_STDIN    'I'

struct probuf {
    char pletter;   /* The letter of the protocol */
    char *pname;    /* Descriptive name of protocol */
    /* The follwing are loaded from PF flags in the 3rd field */
    int
    pbatch,     /* Non-zero if this is a batch type protocol */
    pstdin;     /* Does the transfer protocol program accept
               stdin as the file to transfer as - */
    int pbsize; /* Transfer block size */
    int pbjunk; /* Relative overhead factor for this protocol */
    char *pamsg;    /* Message to print to tell how to abort */
    char *plfop;    /* The option to be given before a log file name */
    char *pscmd;    /* The program to run to send files */
    char *prcmd;    /* The program to run to receive files */
};

/* Default protocol file name */
#define PROTOFILE   "protos"    /* In SKYDIR/lib */
