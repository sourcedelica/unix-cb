unix-cb
=======

Unix-CB, a chat system from the olden days

Unix-CB (or CB for short) was an attempt to bring CB (Citizen's Band radio) to the computer.  You could hang out on a channel and talk with your friends.  If someone was annoying you could squelch them.

CB was originally written in Fortran for an IBM/370 mainframe running [MUSIC](http://en.wikipedia.org/?title=MUSIC/SP).  It was then ported to Unix and C, first running on a Convergent Technologies [Miniframe](http://sourcedelica.com/blog/wp-content/uploads/2014/11/convergent-miniframe.png) and then on SCO Xenix, an early System V-on-80386.

CB was part of Skynet, a multi-line BBS package. [Morrison Hotel](http://bbslist.textfiles.com/714/oldschool.html).  Morrison Hotel was the first (and regrettably, only) Skynet BBS, based in Orange County, CA in the mid-late 80s

A number of chat systems based on the CB user interface were developed in the 90s, including [Unix-CB](https://github.com/ggrossman/unix-cb) by Gary Grossman (which was used by the hugely popular [Vrave](http://hyperreal.org/raves/vrave/)).  If you want to see what it was like there is a running replica called Enormous Trousers.  At your command line run `telnet chat.f4.ca 6623`.

Unix-CB uses System V IPC for communication, shared memory and synchonization.  It splits into two processes, one for 
input and one for output (there were no threads back in those days).  While input is happening it raises a semaphore to 
block the output.  It also uses the semaphore to synchronize updates to shared memory.

I put the code up on GitHub for sentimental reasons. I would have put up all of the code for Skynet but unfortunately it appears to be lost forever...   

## Contributors
- Gary Grossman got the code in shape so it would compile cleanly with modern compilers and lexers.

## To Do
- Remove hardcoded `DEFBASE`.  Replace using relative path(s) and `configure` or similar.
- Get dorky chat working.  Have CB put tty device name in each slot instead of getting it from Skynet.

## Installation
Make sure you have the flex library installed.  For example, `yum install flex`.

### Compiling

    cd sub
    make
    cd ../cb
    # edit cb.h, set DEFBASE to the root of this project
    make

### Running the first time

    cb/cbinit
    
### To run CB

    export PAID=T
    cb/cb

Enjoy!
