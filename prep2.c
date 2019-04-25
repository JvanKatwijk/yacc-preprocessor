
#
#include	<stdio.h>
#include	"prep.h"

/*
 *	J van Katwijk
 *	Delft University of Technology
 *	Faculty of Mathematics and Informatics
 *	132 julianalaan, Delft
 *	The Netherlands
 *
 *	Scanning/copying/parsing routines for the
 *	YACC preprocessor
 */

static char c;

static	int	is_digit (x)
char	x;
{
	return '0' <= x && x <= '9';
}

static	int	is_letter (x)
char	x;
{	return 'a' <= x && x <= 'z' ||
               'A' <= x && x <= 'Z';
}

int	is_namelem (x)
int	x;
{
	return is_letter (x) ||
	       x == '_'      ||
               x == '.'      ||
               x == '$';
}

int	is_control (x)
int	x;
{
	return 0 <= x && x <= ' ';
}


int	gettok ()
{
	int	i,
		base;
	static int	peekline = 0;
	int	match,
		reserve;

/*
 */
begin:

	reserve = 0;

	lineno += peekline;
        peekline = 0;

	c = fgetc (finput);
	while (is_control (c)) {
           if (c == '\n') lineno ++;
           c = fgetc (finput);
        }

	if (c == '/') {  /* go for comment */
           lineno += skipcom ();
           goto begin;
        }

	switch (c) {
	   case EOF:
	        return ENDFILE;
	   case ':':
	        return COLON;
	   case ';':
	        return SEMI;
	   case '|':
	        return BAR;
	   case '{':
	        ungetc (c, finput);
                return (EQ_SYMB);
	        /* .... action .... */
	   case '<':	/* Only for YACC */
	        i = 0;
                while ( (c = fgetc (finput)) != '>' &&
                         c >= 0 && c != '\n') {
                   tokname [i] = c;
	           if (++i >= NAMESIZE) --i;
	        }

	        ERROR( (c != '>'),
                    (stderr, "Unterminated <..>"));
	        tokname [i] = '\0';
	        return IDENT;

	   case ',':
	       return COMMA;
	   case '^':
	      return OUTPUT;
	   case '=':
	      return EQ_SYMB;
	   case '(':
	      return LPAR;
	   case ')':
	      return RPAR;
	   case '"':
	   case '\'':
	      match = c;
	      tokname [0] = ' ';
	      i = 1;

	      for (;;) {
	         c = fgetc (finput);
	         ERROR ( (c == '\n' || c == EOF),
	            (stderr, "Illegal or missing %c", match));
	         if (c == '\\') {
	            c = fgetc (finput);
	            tokname [i] = '\\';
                    if (++ i >= NAMESIZE) --i;
                 }
                 else
                    if (c == match)
                       break;
                    
                 tokname [i] = c;
                 if (++i >= NAMESIZE) --i;
              }

              break;

	   case '%':
	   case '\\':
	      switch (c = fgetc (finput)) {
	         case '0':	return TERM;
	         case '<':	return LEFT;
	         case '2':	return BINARY;
	         case '>':	return RIGHT;
	         case '%':
	         case '\\':	return MARK;
	         case '=':	return PREC;
	         case '{':	return LCURL;

	         default:	reserve = 1;
	      }

/*
 *	bah, Johnosn falls through here, we do the same
 */
	   default:
	      if (is_digit (c)) {
                 i = 0;
                 while (is_digit (c)) {
	            tokname [i] = c;
                    if (++ i >= NAMESIZE) --i;
	            c = fgetc (finput);
	         }

	         ungetc (c, finput);
	         return NUMBER;
	      }

	      if (is_namelem (c)) {
	         i = 0;
	         while (is_namelem (c) || is_digit (c) ) {
	            tokname [i] = c;
	            if (reserve && isupper (c))
	               tokname [i] += 'a' - 'A';
	            if (++ i >= NAMESIZE) --i;
	            c = fgetc (finput);
	         }
	      }
	      else
	         return c;

	      ungetc (c, finput);
	}

/*
 *	look for reserved words
 */
	tokname [i] = '\0';

	if (reserve) {
	   if (!strcmp (tokname, "term")) return TERM;
	   if (!strcmp (tokname, "token")) return TERM;
	   if (!strcmp (tokname, "left")) return LEFT;
	   if (!strcmp (tokname, "nonassoc")) return BINARY;
	   if (!strcmp (tokname, "binary")) return BINARY;
	   if (!strcmp (tokname, "right")) return RIGHT;
	   if (!strcmp (tokname, "prec")) return RIGHT;
	   if (!strcmp (tokname, "start")) return START;
	   if (!strcmp (tokname, "type")) return TYPE;
	   if (!strcmp (tokname, "union")) return UNION;
	   if (!strcmp (tokname, "attributes")) return ATTR_DEF;
	   if (!strcmp (tokname, "attribute")) return ATTR_DEF;
	   ERROR (TRUE, (stderr, "non-recognized keyword %s", tokname));
	}
	return IDENT;
}

/*
 *	Copy the union
 */

void	cpyunion () {
int	level,
	c;

	fprintf (foutput, "\n /*# line %d \"%s\"*/\n", lineno, infile);
	fprintf (foutput, "%%union");

	level = 0;
	for (;;) {
	   if ( (c = fgetc (finput)) < 0) {
	      ERROR (TRUE, (stderr,  "EOF on parsing union"));
	   }

	   fprintf (foutput, "%c", c);
	   switch (c) {
	      case '\n': ++ lineno; break;
	      case '{':
	                 ++ level; break;
	      case '}':
	                 level --;
	                 if (level == 0) return;
	      default:
		       ; /* do nothing */
	   }
	}
}


void	cpycode ()	/* copy the code between { and } */
{
	int	c = fgetc (finput);

	if (c == '\n') {
	   c = fgetc (finput);
	   lineno ++;
	}

	fprintf (foutput, "\\{");
	while (c > 0) {
	   if (c == '\\') {
	      if ((c = fgetc (finput)) == '}') {
	         fprintf (foutput, "\\}");
	         return;
	      }
              else
	         fprintf (foutput, "\\");
	   }

	   if (c == '%') {
	      if ((c = fgetc (finput)) == '}') {
	         fprintf (foutput, "%}");
	         return;
	      }
	      else
	         fprintf (foutput, "%");
	   }

	   fprintf (foutput, "%c", c);
	   if (c == '\n') lineno ++;
	   c = fgetc (finput);
	}

	ERROR (TRUE, (stderr, "Error before %%}"));
}

int	skipcom ()
{
	int	c;
	int	i = 0;	/* number of lines skipped */

	ERROR ( (fgetc (finput) != '*'), (stderr, "Illegal comment"));

	c = fgetc (finput);
	while (c != EOF) {
	   while (c == '*') 
	      if ((c = fgetc (finput)) == '/') return i;

	   if (c == '\n') i ++;
	   c = fgetc (finput);
	}

	ERROR (TRUE, (stderr, "Error EOF within comment"));
}

void	cpyact ()	/* copy C code to closing ; or } */
{
	int	brac,
		c,
		match,
		j,
		s,
		tok;

	pref_stackadjust ();
	brac = 0;

loop:
	c = fgetc (finput);

swt:
	switch (c) {
	   case ';':
	      if (brac == 0) {
	         post_adjuststack ();
	         fprintf (foutput, "%c", c);
	         return;
	      }
	      goto lcopy;

	   case '{':
	      brac ++; goto lcopy;

	   case '$':
	      s = 1;
	      tok = -1;
	      c = fgetc (finput);
	      if (c == '<') {
	         ungetc (c, finput);
	         ERROR( (gettok () != IDENT),
	            (stderr, "Bad syntax on $<...>"));
	         fprintf (foutput, "<%s>", tokname);
	         c = fgetc (finput);
	      }

	      if (c == '$') {
	         fprintf (foutput, "$$");
	         goto loop;
	      }

	      if (c == '-') {
	         s = -s;
	         c = fgetc (finput);
	      }
	      if (is_digit (c)) {
	         j = 0;
	         while (is_digit (c)) {
	               j = 10 * j + c - '0';
	               c = fgetc (finput);
	         }

	         j = s * j;
	         fprintf (foutput, "$%d", j);
	         goto swt;
	      }

	      if (is_letter (c)) {
	         ungetc (c, finput);
	         tok = gettok ();
	         ERROR ( (tok != IDENT),
	            (stderr, "Illegal $ construct"));
	         fprintf (foutput, "%s", address_of (tokname));
	         goto loop;
	      }

	      fprintf (foutput, "$");
	      if (s < 0) fprintf (foutput, "-");

	      goto swt;

	   case '}':
	      if (-- brac > 0) goto lcopy;

	      post_adjuststack ();
	      fprintf (foutput, "%c", c);
	      return;

	   case '/':	/* look for comment */
	      fprintf (foutput, "%c", c);
	      c = fgetc (finput);
	      if (c != '*') goto swt;

	      while (c != EOF) {
	         while (c == '*') {
	            fprintf (foutput, "%c", c);
	            if ((c = fgetc (finput)) == '/') goto lcopy;
	         }
	         fprintf (foutput, "%c", c);
	         if (c == '\n') lineno ++;
	         c = fgetc (finput);
	      }

	      ERROR (TRUE, (stderr, "EOF within comment"));

	   case '\'':
	      match = '\'';
	      goto string;

	   case '\"':
	      match = '\"';

string:
	      fprintf (foutput, "%c", c);
	      while ((c = fgetc (finput)) != EOF) {
	         if (c == '\\') {
	            fprintf (foutput, "%c", c);
	            c = fgetc (finput);
	            if (c == '\n') lineno ++;
	         }
	         else
	         if (c == match) goto lcopy;
	         else
	         ERROR ( (c == '\n'),
	            (stderr, "Newline in string"));
                 fprintf (foutput, "%c", c);
	      }

	      ERROR (TRUE, (stderr, "EOF in string or character constant"));

	   case EOF:
	      ERROR (TRUE, (stderr, "action does not terminate"));

	   case '\n':
	      lineno ++;
	      goto lcopy;
	}

lcopy:
	fprintf (foutput, "%c", c);
	goto loop;
}


	  



