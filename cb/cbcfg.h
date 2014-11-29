/*	Unix CB
	
	Configuration functions definitions
*/

#ifndef CCBCFG
extern int URESSIZ, FSIZE, CBKEY, MAXPOUT, ICTLVAL, MAXUOUT, LOGCOPAS;
extern char *RECNAME, *LFCHAR, *FAKEOUT, *AFORDEF, *FORDEF, *CBINIT;
extern int HOMEREC, MAXCHAN, LIMCHAN, LIMBASE, LIMIT, MONCHAN, MONPAY;
extern int MONBASE, MONLIM, PAIDLF, PAIDDC, MAXNL, MAXMYOUT, NUMSLOTS;
extern char *DOSHELL, *DOFORM, *DOINACT, *DOTYPE, *DODORK, *CBLOGIN;
extern char *LOCKEDMSG, *CMDFILE, *CBDIR, *RECDIR, *OUTFILE, *CBHELP;
extern int INITCHAN, PINITCHAN, UINITCHAN, RCLOW, RCHIGH, FORCEH, FORCEL;
extern int AFORCEH, AFORCEL, WRITELEV, STTY0, KPERCENT, QUOTA, COKGLOB;
extern char *MSGPBAD, *MSGSBAD, *MSGVBAD, *MSGCBAD, *MSGBAD, *DAILY;
extern char *ENVPAID, *ENVSYSOP, *ENVCOSYSOP, *ENVVIP, *ENVVALID, *UNIXWHO;
extern int COACT, COPASSX, COONE, COPPA, COLOCKOV, TSINTERVAL, CONEGV;
extern int COKINACT, COKPAID, DEBUG;

extern char *cmdbads[];
extern int cmdops[];
extern struct cmdnode *cmdlist;
#endif

struct cmdnode {
	char type;
	char *acts;
	char *desc;
	int index;
	struct cmdnode *next;
};
#define CNNULL	(struct cmdnode *)NULL

/* Command array index locations */
#define CI_REG		0
#define CI_PAID		1
#define CI_COSYSOP	2
#define CI_SYSOP	3
#define CI_VIP		4

extern char *cbfn(), *cfgxstr(), *cbrfn(), *cbqfn(), *basepath();
