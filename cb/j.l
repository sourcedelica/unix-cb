%e 1800
%p 3000
%n 1000
%k 500
%a 2000
%o 1000
BW [    ]
EW [    .,;!?]

%{
    char buf[128];
    extern char *cbjs, *cbjt;

#undef input
#define input()     cbjin()
#undef unput
#define unput(x)    cbjun(x)

%}

%%
file                produce(" stash");
send                produce(" t'row");
program             produce(" honky code");
atlas               produce(" Isaac");
unix                produce(" slow mo-fo");
UNIX                produce(" that slow mo-fo");
takes               produce(" snatch'd");
Mexican             produce(" wet-back");
mexican             produce(" wet-back");
Italian             produce(" greaser");
italian             produce(" greaser");
take                produce(" snatch");
"don't"             produce(" duzn't");
jive                produce(" JIBE");
fool                produce(" honkyfool");
modem               produce(" doodad");
"e the "            produce("e da damn ");
"a the "            produce("a da damn ");
"t the "            produce("t da damn ");
"d the "            produce("d da damn ");
" man "                     produce(" dude ");
woman               produce("mama");
girl                produce("goat");
something           produce("sump'n");
" lie "         produce(" honky jibe ");
-o-                     produce(" -on rebound- ");
-oo-            produce(" -check y'out latah-");
[a-b]"."        { sprintf(buf, "%s  Sheeeiit.",yytext); produce(buf); }
[e-f]"."        { sprintf(buf, "%s  What it is, Mama!",yytext); produce(buf); }
[i-j]"."        { sprintf(buf, "%s  Ya' know?",yytext); produce(buf); }
[m-n]"."        { sprintf(buf, "%s  'S coo', bro.",yytext); produce(buf); }
[q-r]"."        { sprintf(buf, "%s  Ah be baaad...",yytext); produce(buf); }
[u-v]"."        { sprintf(buf, "%s  Man!",yytext); produce(buf); }
[y-z]"."        { sprintf(buf, "%s  Slap mah fro!",yytext); produce(buf); }
Sure            produce("Sho' nuff");
sure            produce("sho' nuff");
\40+get             produce(" git");
"will have"         produce("gots'ta");
"will "             produce("gots'ta ");
"got to"            produce("gots'ta");
"I am"              produce("I's gots'ta be");
"am not"                produce("aint");
"is not"                produce("aint");
"are not"               produce("aint");
" are your"             produce(" is yo'");
" are you"              produce(" you is");
\40+hat\40+                     produce(" fedora ");
\40+shoe                    produce(" kicker");
haven't             produce("aint");
"have to"           produce("gots'ta");
have                produce("gots'");
" has"              produce(" gots'ta");
"come over"         produce("mosey on down");
\40+come\40+                    produce(" mosey on down ");
!                       produce(".  Right On!  ");
buy             produce("steal");
\40+car\40+                     produce(" wheels ");
drive               produce("roll");
\40+eat\40+                     produce(" feed da bud ");
\40+black                   produce(" brother ");
\40+negro                   produce(" brother");
white               produce("honky");
\40+nigger                  produce(" gentleman ");
nice                produce("supa' fine");
"person"            produce("sucka'");
\40+thing                   produce(" wahtahmellun");
home                    produce("plantation");
name                produce("dojigger");
\40+path                    produce(" alley");
computer            produce("clunker");
or              produce("o'");
killed              produce("wasted");
president           produce("super-dude");
"prime minister"        produce("super honcho");
injured             produce("hosed");
government          produce("guv'ment");
knew                produce("knowed");
because             produce("a'cuz");
Because             produce("A'cuz");
your                produce("yo'");
Your                produce("Yo'");
four                produce("foe");
got             produce("gots");
aren't              produce("ain't");
young               produce("yung");
you             produce("ya'");
You             produce("You's");
first               produce("fust");
police              produce("honky pigs");
\40+string                  produce(" chittlin'");
\40+read        produce(" eyeball");
write               produce("scribble");
th              produce("d");
Th              produce("D");
ing             produce("in'");
\40+a\40+           produce(" some ");
\40+to\40+          produce(" t'");
tion                produce("shun");
\40+almost\40+          produce(" mos' ");
" from"         produce(" fum");
\40+because\40+     produce(" cuz' ");
you're      produce("youse");
You're      produce("Youse");
alright         produce("coo'");
okay            produce("coo'");
"er "           produce("a' ");
known           produce("knode");
want            produce("wants'");
beat            produce("whup'");
exp         produce("'sp");
exs         produce("'s");
" exc"          produce(" 's");
" ex"           produce(" 'es");
like            produce("likes");
did             produce("dun did");
"kind of"           produce("kind'a");
women               produce("honky chicks");
" men "             produce(" dudes ");
" mens "            produce(" dudes ");
" man "             produce(" dude ");
woman               produce("honky chick");
dead            produce("wasted");
good            produce("baaaad");
"open "             produce("jimmey ");
"opened "           produce("jimmey'd ");
" very"             produce(" real");
"per"               produce("puh'");
"pera"              produce("puh'");
"oar"               produce("o'");
" can"              produce(" kin");
"just "             produce("plum ");
detroit         produce("Mo-town");
"western electric"      produce("da' cave");
" believe"          produce(" recon'");
[Ii]"ndianapolis"       produce("Nap-town");
" "[Jj]"ack"            produce(" Buckwheat");
" "[Bb]"ob "        produce(" Liva' Lips ");
" "[Pp]"hil "       produce(" dat fine soul ");
" "[Mm]"ark "       produce(" Amos ");
[Rr]obert       produce("Leroy");
[Ss]"andy"      produce("dat fine femahnaine ladee");
[Jj]"ohn "      produce("Raz'tus ");
" "[Pp]"aul"        produce(" Fuh'rina");
[Rr]"eagan"     produce("Kingfish");
[Dd]"avid"      produce("Issac");
[Rr]"onald"     produce("Rolo");
" "[Jj]"im "        produce(" Bo-Jangles ");
" "[Mm]"ary"        produce(" Snow Flake");
[Ll]"arry"      produce("Remus");
[Jj]"oe"        produce("Massa' ");
[Jj]"oseph"     produce("Massa' ");
mohammed            produce("liva' lips");
pontiff             produce("wiz'");
pope                produce("wiz'");
pravda              produce("dat commie rag");
broken              produce("bugger'd");
"strange "          produce("funky ");
"dance "            produce("boogy ");
" house"            produce(" crib");
ask             produce("ax'");
" so "              produce(" so's ");
head                produce("'haid");
boss                produce("main man");
wife                produce("mama");
people              produce("sucka's");
money               produce("bre'd");
amateur             produce("begina'");
radio                   produce("transista'");
" of "              produce(" uh ");
what                produce("whut");
does                produce("duz");
was             produce("wuz");
" were"             produce(" wuz");
"understand it"         produce("dig it");
understand          produce("dig it");
" my"               produce(" mah'");
" "[Ii]" "          produce(" ah' ");
"meta"              produce("meta-fuckin'");
"hair"          produce("fro");
"talk"          produce("rap");
"music"         produce("beat");
"basket"        produce("hoop");
"football"      produce("ball");
"friend"        produce("homey");
"school"        produce("farm");
"boss"          produce("Man");
"want to"       produce("wanna");
"wants to"      produce("be hankerin' aftah");
"well"          produce("sheeit");
"Well"          produce("Sheeit");
"big"           produce("big-ass");
"bad"           produce("bad-ass");
"small"         produce("little-ass");
"sort of"       produce("radical");
" is "          produce(" be ");
water           produce("booze");
book            produce("scribblin'");
magazine        produce("issue of GQ");
paper           produce("sheet");
up          produce("down");
down            produce("waaay down");
break           produce("boogie");
Hi          produce("'Sup, dude");
VAX         produce("pink Cadillac");
.           produce(yytext);
\n          produce("\n");

%%

cbjin()
{
    return( *(cbjs++) );
}

cbjun(c)
int c;
{

    *(--cbjs) = c;
}

produce( s )
char *s;
{
    strcat( cbjt, s );
}
