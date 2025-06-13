/* tinytcap.c */

/* This file contains functions which simulate the termcap functions.
 *
 * It doesn't access a "termcap" file.  Instead, it uses an initialized array
 * of strings to store the entries.  Any string that doesn't start with a ':'
 * is taken to be the name of a type of terminal.  Any string that does start
 * with a ':' is interpretted as the list of fields describing all of the
 * terminal types that precede it.
 *
 * Note: since these are C strings, you can't use special sequences like
 * ^M or \E in the fields; your C compiler won't understand them.  Also,
 * at run time there is no way to tell the difference between ':' and '\072'
 * so I sure hope your terminal definition doesn't require a ':' character.
 *
 * Note that you can include several terminal types at the same time.  Elvis
 * chooses which entry to use at runtime, based primarily on the value of $TERM.
 * Exception: getenv(TERM) on VMS checks the SET TERM device setting.  To
 * implement non-standard terminals set the logical ELVIS_TERM in VMS. (jdc)
 */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_tinytcap[] = "$Id: tinytcap.c,v 2.27 2004/01/30 23:53:18 steve Exp $";
#endif
#ifdef NEED_TGETENT
# include <stdlib.h>
# include <stdio.h>	/* for 'sprintf()' */

#define TRACE(x)

#if USE_PROTOTYPES
static char *find(char *id, int vtype);
#endif

short ospeed;

/*ARGSUSED*/
int tgetent(bp, name)
	char	*bp;	/* buffer for storing the entry -- ignored */
	char	*name;	/* name of the entry */
{
	return 1;
}

int tgetnum(id)
	char	*id;
{
    serial_debug("tgetnum(%s)\n", id);

    if (strcmp(id, "co") == 0) {
        return 80;
    } else if (strcmp(id, "li") == 0) {
        return 24;
    }

    return -1;
}

int tgetflag(id)
	char	*id;
{
	serial_debug("tgetflag(%s)\n", id);
    return 0;
}

/*ARGSUSED*/
char *tgetstr(id, bp)
	char	*id;
	char	**bp;	/* pointer to pointer to buffer - ignored */
{
	serial_debug("tgetstr(%s)\n", id);
    if (strcmp(id, "up") == 0) {
        return "\033[A";  /* ANSI escape sequence for cursor up */
    } else if (strcmp(id, "do") == 0) {
        return "\033[B";  /* ANSI escape sequence for cursor down */
    } else if (strcmp(id, "nd") == 0) {
        return "\033[C";  /* ANSI escape sequence for cursor right */
    } else if (strcmp(id, "le") == 0) {
        return "\033[D";  /* ANSI escape sequence for cursor left */
    } else if (strcmp(id, "cl") == 0) {
        return "\033[2J\033[H"; /* ANSI clear screen and home cursor */
    } else if (strcmp(id, "cm") == 0) {
        return "\033[%d;%dH"; /* ANSI cursor movement */
    } else if (strcmp(id, "ce") == 0) {
        return "\033[K"; /* ANSI clear to end of line */
    }

    return 0;
}

/*ARGSUSED*/
char *tgoto(cm, destcol, destrow)
	char	*cm;	/* cursor movement string */
	int	destcol; /* destination column, 0 - 79 */
	int	destrow; /* destination row, 0 - 24 */
{
    serial_debug("tgoto(%s, %d, %d)\n", cm, destcol, destrow);

	static char buf[30];
	char	*build;
	int	tmp;

	for (build = buf; *cm; cm++)
	{
		if (*cm == '%')
		{
			switch (*++cm)
			{
			  case '+':
				tmp = destrow;
				destrow = destcol;
				destcol = tmp;
				*build++ = *++cm + tmp;
				break;

			  case 'i':
				destcol++;
				destrow++;
				break;

			  case 'd':
				tmp = destrow;
				destrow = destcol;
				destcol = tmp;
				sprintf(build, "%d", tmp);
				build += strlen(build);
				break;
			}
		}
		else
		{
			*build++ = *cm;
		}
	}
	*build = '\0';
	return buf;
}

/*ARGSUSED*/
void tputs(cp, affcnt, outfn)
	char	*cp;		/* the string to output */
	int	affcnt;		/* number of affected lines -- ignored */
	int	(*outfn) P_((int));	/* the output function */
{

	while (*cp)
	{
		(*outfn)(*cp);
		cp++;
	}
}
#endif /* NEED_TGETENT */
