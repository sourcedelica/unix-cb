#if HAVE_TERMIOS_H

#include <termios.h>

#define TERMIO_OR_TERMIOS struct termios

#elif HAVE_TERMIO_H

#include <termio.h>

#define TERMIO_OR_TERMIOS struct termio

#define tcgetattr(fd, save) ioctl(fd, TCGETA, save)
#define tcsetattr(fd, optional_actions, save) ioctl(fd, TCSETAW, save)

#else

struct termio_dummy {};
#define TERMIO_OR_TERMIOS struct termio_dummy

#define tcgetattr(fd, save)
#define tcsetattr(fd, optional_actions, save)

#endif

#ifndef L_cuserid
#define L_cuserid 256
#endif

void saveterm(TERMIO_OR_TERMIOS *save);
void restterm(TERMIO_OR_TERMIOS *save);

#ifdef __linux__
/* On Linux, you must define this yourself. */
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};
#endif
