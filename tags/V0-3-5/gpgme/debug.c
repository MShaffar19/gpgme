/* debug.c
 *      Copyright (C) 2001 g10 Code GmbH
 *
 * This file is part of GPGME.
 *
 * GPGME is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPGME is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#ifndef HAVE_DOSISH_SYSTEM
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
#endif
#include <assert.h>

#include "util.h"
#include "sema.h"

DEFINE_STATIC_LOCK (debug_lock);

struct debug_control_s {
    FILE *fp;
    char fname[100];
};

static int debug_level = 0;
static FILE *errfp = NULL;

/****************
 * remove leading and trailing white spaces
 */
static char *
trim_spaces( char *str )
{
    char *string, *p, *mark;

    string = str;
    /* find first non space character */
    for( p=string; *p && isspace( *(byte*)p ) ; p++ )
	;
    /* move characters */
    for( (mark = NULL); (*string = *p); string++, p++ )
	if( isspace( *(byte*)p ) ) {
	    if( !mark )
		mark = string ;
	}
	else
	    mark = NULL ;
    if( mark )
	*mark = '\0' ;  /* remove trailing spaces */

    return str ;
}


static void
debug_init (void)
{
    static volatile int initialized = 0;
       
    if (initialized) 
        return;
    LOCK (debug_lock);
    if (!initialized) {
        const char *e = getenv ("GPGME_DEBUG");
        const char *s1, *s2;;

        initialized = 1;
        debug_level = 0;
        errfp = stderr;
        if (e) {
            debug_level = atoi (e);
            s1 = strchr (e, ':');
            if (s1 
#ifndef HAVE_DOSISH_SYSTEM
                && getuid () == geteuid ()
#endif
                ) {
                char *p;
                FILE *fp;

                s1++;
                if ( !(s2 = strchr (s1, ':')) )
                    s2 = s1 + strlen(s1);
                p = xtrymalloc (s2-s1+1);
                if (p) {
                    memcpy (p, s1, s2-s1);
                    p[s2-s1] = 0;
                    trim_spaces (p);
                    fp = fopen (p,"a");
                    if (fp) {
                        setvbuf (fp, NULL, _IOLBF, 0);
                        errfp = fp;
                    }
                    xfree (p);
                }
            }
        }

        if (debug_level > 0)
            fprintf (errfp,"gpgme_debug: level=%d\n", debug_level);
    }
    UNLOCK (debug_lock);
}

int
_gpgme_debug_level ()
{
    return debug_level;
}

void
_gpgme_debug (int level, const char *format, ...)
{
    va_list arg_ptr ;

    debug_init ();
    if ( debug_level < level )
        return;
    
    va_start ( arg_ptr, format ) ;
    LOCK (debug_lock);
    vfprintf (errfp, format, arg_ptr) ;
    va_end ( arg_ptr ) ;
    if( format && *format && format[strlen(format)-1] != '\n' )
        putc ('\n', errfp);
    UNLOCK (debug_lock);
    fflush (errfp);
}



void
_gpgme_debug_begin ( void **helper, int level, const char *text)
{
    struct debug_control_s *ctl;

    debug_init ();

    *helper = NULL;
    if ( debug_level < level )
        return;
    ctl = xtrycalloc (1, sizeof *ctl );
    if (!ctl) {
        _gpgme_debug (255, __FILE__ ":" STR2(__LINE__)": out of core");
        return;
    }

    /* Oh what a pitty that we don't have a asprintf or snprintf under
     * Windoze.  We definitely should write our own clib for W32! */
    sprintf ( ctl->fname, "/tmp/gpgme_debug.%d.%p", getpid (), ctl );
  #if defined (__GLIBC__) || defined (HAVE_DOSISH_SYSTEM)
    ctl->fp = fopen (ctl->fname, "w+x");
  #else 
    {
        int fd  = open (ctl->fname, O_WRONLY|O_TRUNC|O_CREAT|O_EXCL,
                        S_IRUSR|S_IWUSR );
        if (fd == -1)
            ctl->fp = NULL;
        else
            ctl->fp = fdopen (fd, "w+");
    }
  #endif
    if (!ctl->fp) {
        _gpgme_debug (255,__FILE__ ":" STR2(__LINE__)": failed to create `%s'",
                      ctl->fname );
        xfree (ctl);
        return;
    }
    *helper = ctl;
    _gpgme_debug_add (helper, "%s", text );
}

int
_gpgme_debug_enabled (void **helper)
{
    return helper && *helper;
}


void
_gpgme_debug_add (void **helper, const char *format, ...)
{
    struct debug_control_s *ctl = *helper;
    va_list arg_ptr ;

    if ( !*helper )
        return;
    
    va_start ( arg_ptr, format ) ;
    vfprintf (ctl->fp, format, arg_ptr) ;
    va_end ( arg_ptr ) ;
}

void
_gpgme_debug_end (void **helper, const char *text)
{
    struct debug_control_s *ctl = *helper;
    int c, last_c=EOF;

    if ( !*helper )
        return;
    
    _gpgme_debug_add (helper, "%s", text );
    fflush (ctl->fp); /* we need this for the buggy Windoze libc */
    rewind (ctl->fp);
    LOCK (debug_lock);
    while ( (c=getc (ctl->fp)) != EOF ) {
        putc (c, errfp);
        last_c = c;
    }
    if (last_c != '\n')
        putc ('\n', errfp);
    UNLOCK (debug_lock);
    
    fclose (ctl->fp);
    remove (ctl->fname);
    xfree (ctl);
    *helper = NULL;
}
