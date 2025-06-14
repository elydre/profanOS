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

#include <profan/panda.h>
#include <profan.h>

int tgetnum(id)
	char	*id;
{

    int x, y;

    if (strcmp(id, "co") == 0) {
        panda_get_size(&x, &y);
        return x;
    } else if (strcmp(id, "li") == 0) {
        panda_get_size(&x, &y);
        return y;
    }

    serial_debug("tgetnum(%s)\n", id);

    return -1;
}

int tgetflag(id)
	char	*id;
{
	serial_debug("tgetflag(%s)\n", id);
    return 0;
}

typedef struct {
    const char *key;
    const char *ansi_sequence;
} TermcapMapping;

TermcapMapping termcap_mappings[] = {
    "up", "\033[A",  // up one line
    "cm", "\033[%d;%dH",  // cursor movement to row N, col M
    "ce", "\033[K",  // clear to end of line
    "do", "\033[B",  // down one line

    "bc", "\033[D",  // backspace
    "vb", NULL,  // visible bell
    "DO", "\033[%dB",  // down N lines
    "ti", NULL,  // terminal initialization
    "te", NULL,  // terminal end
    "sc", "\033[s",  // save cursor position
    "rc", "\033[u",  // restore cursor position
    "ks", NULL,  // start function keys
    "ke", NULL,  // end function keys
    "AF", "\033[%dm",  // set foreground color
    "sg", NULL,  // ?
    "so", NULL,  // standout mode
    "se", NULL,  // end standout mode
    "ug", NULL,  // underline magic (?)
    "us", "\033[4m",   // start underline
    "ue", "\033[24m",  // end underline
    "me", "\033[0m",   // end all modes
    "IC", NULL,  // insert character
    "ic", NULL,  // insert character
    "DC", NULL,  // delete character
    "dc", NULL,  // delete character
    "AL", NULL,  // insert blank line
    "al", NULL,  // insert blank line
    "DL", NULL,  // delete line
    "dl", NULL,  // delete line
    "sr", NULL,  // screen scroll
    "sC", NULL,  // ?
    "ve", NULL,  // normal cursor
    "vs", NULL,  // enhanced cursor
    "kl", "\033[D",  // cursor left
    "kr", "\033[C",  // cursor right
    "ku", "\033[A",  // cursor up
    "kd", "\033[B",  // cursor down
};

/*ARGSUSED*/
char *tgetstr(id, bp)
	char	*id;
	char	**bp;	/* pointer to pointer to buffer - ignored */
{
	
    for (int i = 0; i < sizeof(termcap_mappings) / sizeof(TermcapMapping); i++) {
        if (strcmp(termcap_mappings[i].key, id) == 0) {
            return (char *)termcap_mappings[i].ansi_sequence;
        }
    }

    serial_debug("tgetstr(%s)\n", id);

    return 0;
}

/*ARGSUSED*/
char *tgoto(cm, destcol, destrow)
	char	*cm;	/* cursor movement string */
	int	destcol; /* destination column, 0 - 79 */
	int	destrow; /* destination row, 0 - 24 */
{
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
