

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
 *	Process the declaration section of a YACC source
 */

FILE	*finput, *foutput;
char	*infile;
char	tokname [NAMESIZE];
int	tok;
int	lineno	= 1;


void	prep_decls ()
{
	for (tok = gettok (); tok != MARK && tok != ENDFILE; ) {
	   switch (tok) {
	      case SEMI:
	         fprintf (foutput, ";\n");
	         tok = gettok ();
	         break;

	      case START:
	         ERROR ((tok = gettok ()) != IDENT,
		       (stderr, "Bad start construction (YACC)" ));

	         fprintf (foutput, "\n\n%%start %s\n", tokname);
	         tok = gettok ();
	         continue;

	      case TYPE:
	         ERROR ((tok = gettok ()) != IDENT,
	               (stderr, "Bad syntax in %%type (YACC)"));

	         fprintf (foutput, "\n\n%%type <%s> ", tokname);
	         while (TRUE) {
	            tok = gettok ();
	            switch (tok) {
	               case IDENT:
	                  fprintf (foutput, " %s", tokname);
	                  continue;

	               case COMMA:
	                  fprintf (foutput, " ,");
	                  continue;

	               case SEMI:
	                  fprintf (foutput, ";\n");
	                  tok = gettok ();
	                  break;

	               default:
	                  fprintf (stderr, "prep_decls type default %d" , tok);
	                  break;
	            }
	            break;
	         }
	         continue;

	      case ATTR_DEF:
	         proc_def ();
	         continue;

	      case UNION:
	         /* copy union declaration to output */
	         cpyunion ();
	         tok = gettok ();
	         continue;

	      case LEFT:
	      case RIGHT:
	      case BINARY:
	      case TERM:
	         fprintf (foutput, "\n");
	         switch (tok) {
	            case LEFT:
	               fprintf (foutput, "%%left");
	               break;

	            case RIGHT:
	               fprintf (foutput, "%%right");
	               break;

	            case BINARY:
	               fprintf (foutput, "%%nonassoc");
	               break;

	            case TERM:
	               fprintf (foutput, "%%term");
	               break;
	         }

	         /* get identifiers so defined */
	         tok = gettok ();
	         if (tok == IDENT) {
	            fprintf (foutput, " %s", tokname);
	            tok = gettok ();
	         }

	         while (TRUE) {
	            switch (tok) {
	               case COMMA:
	                  fprintf (foutput, " ,");
	                  tok = gettok ();
	                  break;

	               case SEMI:
	                  fprintf (foutput, " ;\n");
	                  break;

	               case IDENT:
	                  fprintf (foutput, " %s", tokname);
	                  tok = gettok ();
	                  break;
	            }
                 break;
	         }
	         continue;

	      case LCURL:
	         cpycode ();
	         tok = gettok ();
	         continue;

	      default:
	         ERROR (TRUE, (stderr, "Syntax error in definition part"));
	   }
	}

	/* tok should not be ENDFILE here: */
	ERROR (tok == ENDFILE, (stderr, "Unexpected EOF before %%"));
}

/*
 *	The rules: that's easy
 */
void	prep_rules ()
{
	fprintf (foutput, "\n%%%%\n");
        tok = gettok ();
        while (tok != ENDFILE && tok != MARK) {
           rule ();
           tok = gettok ();
        }
}

void	prep_post ()
{
	int	c;

	fprintf (foutput, "\n%%%%\n");

        if (tok == MARK)
           while ((c = fgetc (finput)) != EOF)
              fprintf (foutput, "%c", c);

        print_union ();		/* the type */
        fprintf (foutput, "\n /* processed by YACC PREP */ \n");
}

/*
 *	Simple main programme for YACC PREP
 */

void	main (argc, argv)
char	**argv;
int	argc;
{
	int	i;
        int	number_done = 0;

	finput = stdin;
        foutput = stdout;	/* these are the defaults */

        if (argc > 1) {
	   foutput = fopen (argv [1], "w");
        }

	if (foutput == 0) {
	   fprintf (stderr, "Cannot open %s\n", argv [1]);
           exit (3);
        }

        for (i = 2; i < argc; i ++) {
	   infile = argv [i];
           finput = fopen (infile, "r");
           if (finput == 0) {
              fprintf (stderr, "Cannot read from %s\n", infile);
              exit (1);
           }

	   prep_decls ();
	   prep_rules ();

	   prep_post  ();
           number_done ++;
	}

	if (number_done == 0) {
	   infile = "Standard input";
	   finput = stdin;
           
	   prep_decls ();
	   prep_rules ();

	   prep_post  ();
           number_done ++;
	}

	fclose (foutput);
}



