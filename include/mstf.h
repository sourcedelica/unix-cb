/*
 *  Skynet user status flags
 */

#define STF_TAKEN   000000001   /* slot is taken        */
#define STF_SYSTEMSLOT  000000002   /* system program on this slot  */
#define STF_NOPAGE  000000004   /* accept no pages from anyone  */
#define STF_QPAGES  000000010   /* queue up all pages to    */
#define STF_ACTIVE  000000020   /* this slot is currently active*/
#define STF_INGOODPROG  000000040   /* in a "good" program      */
#define STF_POSTING 000000100   /* posting a msg or letter  */
#define STF_NEWPHNMSGS  000000200   /* got new phone msgs       */
#define STF_NEWMAIL 000000400   /* got new mail         */
#define STF_NOWARN  000001000   /* no warning msgs for cutoff   */
#define STF_MSGPEND 000002000   /* msg's que'd up and waiting   */
#define STF_DIALUP  000004000   /* this user is on a modem  */
#define STF_ATPROMPT    000010000   /* this user is at a menu prompt*/
#define STF_CHKULOAD    000020000   /* check the user load      */
#define STF_GRACE   000040000   /* he got his grace for posting */
#define STF_MODIFIED    000100000   /* this record has been changed */
#define STF_WARNED  000200000   /* this guy has been warned */
#define STF_PAID    000400000   /* paid status granted      */
#define STF_NOLIMIT 001000000   /* no time limit for this call  */
