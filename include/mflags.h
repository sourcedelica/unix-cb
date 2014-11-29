/*
 *	User configuarable flags
 */
#define F_EXPERT	00000001	/* do not do help for all menus	*/
#define F_CKPHONEMSGS	00000002	/* check for phone msgs on login*/
#define F_SYSOP		00000004	/* this user is a sysop		*/
#define F_COSYSOP	00000010	/* this user is a co-sysop	*/
#define F_VALIDATED	00000020	/* this user is "validated"	*/
#define F_TICKCREDITS	00000040	/* charge credits for time	*/
#define F_LOGINQUOTE	00000100	/* print a login quote		*/
#define F_LOGOUTQUOTE	00000200	/* print a logout quote		*/
#define F_LOCKED	00000400	/* user locked out		*/
#define F_SHELL		00001000	/* grant shell access		*/
#define F_PAGEBEEP	00002000	/* beep for pages		*/
#define F_MEGABUCKS	00004000	/* call credits megabucks	*/
#define F_VIEWCREDITS	00010000	/* view credits upon login	*/
#define F_AUTOWHO	00020000	/* auto who upon login		*/
#define F_TEMPLOCK	00040000	/* no login while being edited	*/
#define F_NOLIMIT	00100000	/* no time limit		*/
