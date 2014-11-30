/*
 *	Skynet Unix-CB
 *
 *	Definitions
 *
 */

/* Compile-time configuration */

#define STATIC		/* Define blank to have complete name space for adb */

#define DEFBASE		"/home/eric"
#define CBVERS		"1.0"
#define FORMAX		80	/* Max length of format string */
#define CBSERV		1L	/* n > 0 mtype for server requests */
#define L_cbalias	21	/* Max length of handle, plus 1 */
#define ANSPA		"\033[40;1;37m"
#define ANSREG		"\033[40;2;37m"
#define PATHSEP		"/"
#define CBCONFIG	"cbconfig"	/* Default config file */
#define FORMSG		"cbformsg"	/* Format types help */
#define AFORMSG		"cbaformsg"	/* Format types help */
#define BUILTIN		"cbpredef"	/* Predefined formats */
#define ABUILTIN	"cbapredef"	/* Predefined formats */
#define LOGFILE		"cbcolog"	/* PPA log */
#define QUOTADIR	"quotas"	/* Quota files directory */
#define DCPDIR		"lib/dcp"
#define MSGLEN		255	/* Max length of msg */
#define INVALS		"*$#@\007"	/* Invalid characters in handles */
#define GINVALS		"\007"	/* Invalid anywhere in handle */
#define CBMODE		0660	/* Mode for ipcs and log */
#define CBUMASK		077	/* umask setting for fopens */

/* Server request/response types */

#define CB_INVALID	-1

#define CB_LOGIN	1
#define CB_LOGOUT	2
#define CB_ACTIVATE	3
#define CB_DEACT	4
#define CB_CHANDLE	5
#define CB_LOOKUP	6
#define CB_RFPOS	7
#define CB_NEWMSG	8
#define CB_USEREC	9
#define CB_KILL		10
#define CB_UPDATE	11
#define CB_ALIST	12
#define CB_CCHAN	14
#define CB_SQLIST	15
#define CB_SQIT		16
#define CB_GETPOS	17
#define CB_ISSQ		18
#define CB_GETFMT	19
#define CB_SETFMT	20
#define CB_GETOPT	21
#define CB_SETOPT	22
#define CB_NOIN		23
#define CB_INREQ	24
#define CB_LOGOFF	25

/* Options */
/* These haven't been converted to longs in the struct yet.  Any more
   flags and this will be needed. */

#define OP_SQLOCK	0000000001
#define OP_CHLOCK	0000000002
#define OP_LFLOCK	0000000004
#define OP_PMLOCK	0000000010
#define OP_CCLOCK	0000000020
#define OP_SQBEEP	0000000040
#define OP_SYSOP	0000000100
#define	OP_VIP		0000000200
#define	OP_USED		0000000400
#define OP_ACTIVE	0000001000
#define OP_SQNON	0000002000
#define OP_SQPILF	0000004000
#define OP_PAID		0000010000
#define OP_SQLF		0000020000
#define OP_LIST		0000040000
#define OP_MONIT	0000100000
#define OP_ANSI		0000200000
#define OP_SQSTAT	0000400000
#define OP_SKYNET	0001000000
#define OP_JIVE		0002000000
#define OP_COSYSOP	0004000000
#define OP_VALID	0010000000
#define OP_RDOVER	0020000000	/* Read-only override */
#define OP_PABEEP	0040000000
#define OP_UGLYLINK	0100000000

/* Control flag options */

#define CF_LOCKED	000000001
#define CF_RDONLY	000000002
#define CF_VRDONLY	000000004

#define CF_CBICALLED	010000000

/* Structures used */

struct sqrec {
	int sqtype;
};

struct ulrec {
	char userid[L_cuserid];
	char handle[L_cbalias];
	int chan;
	int opts;
	struct sqrec *sqs;
	char format[FORMAX];
	char aformat[FORMAX];
	char doing[FORMAX];
	int lastp;
	int rlastp;
	int kvotes;
	int pid;
	int cpid;
	int towho;
	char lfchar;
	int dcreq;
	int qremain;
};

struct cbreq {
	long mfrom;
	int mfslot;
	int cbtype;
	int arg;
};

struct args {
	int a2;
	int a3;
};

union cbu {
	struct sqrec sq;
	char s[L_cbalias];
	struct args a;
};

struct cball {
	struct cbreq ar;
	union cbu au;
};

struct cbmsg {
	long mtype;
	struct cbreq r;
	union cbu u;
};

#define L_cbreq		sizeof(struct cbreq)
#define L_cbmsg		sizeof(struct cball)

struct dmsg {
	char ufrom[L_cuserid];
	char from[L_cbalias];
	int fslot;
	int tslot;
	char to[L_cbalias];
	int chan;
	int opts;
	char msg[MSGLEN+1];
};

/* Channel enhancements */

#define MSG_PRIV	(MAXCHAN+1)
#define MSG_PA		(MAXCHAN+2)
#define MSG_PPA		(MAXCHAN+3)
#define MSG_STATION	(MAXCHAN+4)
#define MSG_VIP		(MAXCHAN+5)
#define MSG_CPA		(MAXCHAN+6)
#define MSG_NCPA	(MAXCHAN+7)
#define MSG_RPA		(MAXCHAN+8)

/* Squelch types */

#define SQ_PRIV		000001
#define SQ_COMP		000002
#define SQ_BOTH		000003
#define SQ_REVERSE	000004
#define SQ_PASSIVE	000010
#define SQ_STYPE	000017	/* Include all squelch types in this mask */

#define SQ_KILL		001000

#define SQ_SRESET	010000	/* Set squelch to NOBODY */
#define SQ_KRESET	020000	/* Reset kill vote */

/* Globisms */

#define S_INVALID	-101
#define S_NOBODY	-100

#define S_ALL		-99	/* S_ALL must be the lowest numbered global */
#define S_PAID		-98
#define S_NOPAY		-97
#define S_SYSOP		-96
#define S_COSYSOP	-95

#define S_NUMGLOB	5	/* Number of global types */
/* If any more global types are added, be sure to update:
	The gnames array in cbinp.c
	The gmatch function in cbinp.c
*/

/* Shorthand */

#define M_R	mbuf.r
#define M_U	mbuf.u
#define SQNULL	(struct sqrec *)0
#define TRUE	1
#define FALSE	0
#define MYREC	ulog[slot]

/* System dependent defines */

#ifdef UNIX
#define O_BINARY	0
#endif

#ifndef CCB
	extern int slot;
	extern int mid;
	extern long cbpid;
	extern int out;
	extern int done;
	extern int wid;
	extern struct ulrec *ulog;
	extern int cbsem;
	extern int ulsize;
	extern int *pnext, *pcbflag, *pfixslot;
	extern struct sqrec *sqlog;
	extern int child;
	extern int ipcflag;
	extern int sendflag;
	extern int fixsid;
	extern int skynet;
	extern char *cbdoing;
	extern char *cbjs, *cbjt;
	extern int cbjl;
	extern int logging;
#endif

#ifdef NO_SEMUN_DEF
          union semun {
               int val;
               struct semid_ds *buf;
          } arg;
#endif
