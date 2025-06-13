/* tcaposix.c */

#ifdef FEATURE_RCSID
char id_tcaposix[] = "$Id: tcaposix.h,v 2.13 2003/01/26 19:41:36 steve Exp $";
#endif

#include <signal.h>
#include <unistd.h>

/* HPUX does a "#define ttysize winsize".  Elvis doesn't like that. */
#undef ttysize


#if USE_PROTOTYPES
static void catchsig(int signo);
#endif


/* this function is used to catch signals */
static void catchsig(signo)
	int	signo;
{
	caught = (1 << signo);
}

/* get the original tty state */
static void ttyinit2()
{
}

/* switch to the tty state that elvis runs in */
void ttyraw(erasekey)
	char	*erasekey;	/* where to store the ERASE char */
{
}

/* switch back to the original tty state */
void ttynormal()
{
}

/* Read from keyboard with timeout.  For POSIX, we use VMIN/VTIME to implement
 * the timeout.  For no timeout, VMIN should be 1 and VTIME should be 0; for
 * timeout, VMIN should be 0 and VTIME should be the timeout value.
 */
int ttyread(buf, len, timeout)
	char	*buf;	/* where to place the read characters */
	int	len;	/* maximum number of characters to read */
	int	timeout;/* timeout (0 for none) */
{
}
