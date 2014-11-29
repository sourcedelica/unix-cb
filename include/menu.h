#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <termio.h>
#include <skynet/exitcodes.h>
#include <skynet/alias.h>

/*
 *	Skynet menu system header file
 *
 *	You must #define noextern before you #include this header file
 *	if you are using this in your main()
 *
 *	under system V there are 2 copies of this program:
 *		skynet/src/menu/menu.h
 *		/usr/include/skynet/menu.h
 *
 */

/*******************************************************************/

/*
 *	A user record
 */

struct	urec {
		char	uname[L_cuserid], alias[L_alias], device[15],
			doing[65], prompt[80];
		int	fpid, bpid, level, maxtime, days, uload, who;
		long	starttime, from, flags, privs, status, credits;
};

/*
 *	The control struct that sits above the hotel-log in shared
 *	memory.
 */

struct	ctlstr {
		long	sysflags;
		short	paymax, nopaymax, numslots, eatrate, alarmclock, grace,
			almostfull;
		char	skynetpath[80], credits[11], credits2[11], hotkeys[11];
};

struct menopt {
	long privreq;
	int levelreq;
	char option[5], descrip[65], title[65], progopt[9], doing[65],
	**field;
	struct menopt *next;
};

#define icmNIL (struct menopt *) 0

/*
 *	This is a msgqueue type that the background process receives when
 *	something "happens" to the user
 */

struct mymsgbuf {
	long mtype;
	struct actionstr {
		int type;
		char msg[256];
	} act;
} mymsgp;
		
#ifndef noextern

/*
 *	The following takes care of global variable problems.
 *
 */

extern struct urec *hotlog;
extern struct ctlstr *ctl;
extern struct termio oldterm;
extern key_t m_qid, m_mid;
extern int childpid, goodchild, slotnum, ppid;
extern char **recp, **sloginp;
extern long key;

#else

struct urec *hotlog;
struct ctlstr *ctl;
struct termio oldterm;
key_t m_qid, m_mid;
int childpid, goodchild, slotnum, ppid;
char **recp, **sloginp;
long key;
#endif

#define MYREC hotlog[slotnum]

/*
 *	Message types to the background process
 */

#define T_TIMEWARN	1
#define	T_TIMEXP	2
#define	T_SHUTDOWN	3
#define	T_KICKED	4
#define	T_NEWPHONEMSG	5
#define	T_NEWMAIL	6
#define	T_PAGE		7
#define	T_XFERFUNDS	8
#define T_RDQMSGS	9
#define T_SYSMSG	10
#define T_QUIT		11

/*
 *	Skynet user status flags
 */

#include <skynet/mstf.h>

/*
 *	User configurable flags
 */

#include <skynet/mflags.h>

/*
 *	Skynet system status flags
 */

#define S_SYSLOCKED	00000001	/* allow no logins at all	*/
#define S_PAIDONLY	00000002	/* allow only paid users	*/
#define S_GROUPONLY	00000004	/* allow only certain groups	*/
#define S_VALONLY	00000010	/* allow only validated users	*/
#define S_SYSOPONLY	00000020	/* allow only sysops		*/
#define S_COSYSOPONLY	00000040	/* allow only co-sysops		*/
#define S_TICKCREDITS	00000100	/* charge credits		*/
#define S_CKTIME	00000200	/* enforce time limits		*/
#define S_PDGOVERRIDE	00000400	/* paid override on grouponly	*/
#define S_TEMP4		00001000	/* so no recompile necessary	*/

/*
 *	Note: all "privs" are runtime configurable
 */

#define LPCHAR 15
#define SKYUSER	"skynet"
