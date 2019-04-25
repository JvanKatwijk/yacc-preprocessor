
#
/*
 *	prep1.c
 *
 *	J. van Katwijk
 */

#include	<stdio.h>
#include	"prep.h"

/*
 *	rules:
 *	   definitie : attr_def symbol [( ...)]
 *         rule      : lhs : alternative {|alternative}
 *	   alternative : [ {member}] [= action]
 *	   member    : symbol [([|inputlist] [^outputlist])]
 */
/*
 *	local definitions
 */
void	process_intypes ();
void	process_outtypes ();
struct att_def *lhs_side ();
void	rule ();
void	process_alternative ();
void	process_member ();


void	proc_def ()	/* process a definition */
{
	register struct att_def * curr_def;

        ASSERT (tok == ATTR_DEF, "ATTR_DEF expected");

        tok = gettok ();
        ERROR (tok != IDENT, 
                      (stderr, "Syntax error in attribute definition", ""));

        curr_def = new_def (tokname);

        tok      = gettok ();
        if (tok != LPAR)
           return;

        tok = gettok ();
        if (tok == INPUT)
           process_intypes (curr_def);
        if (tok == OUTPUT)
           process_outtypes (curr_def);

        ERROR (tok != RPAR,
                      (stderr, "Syntax error in definition for %s", nonterm_of (curr_def)));

        tok = gettok ();
        return;
}


void	process_intypes (definition)
register struct att_def *definition;
{
	register int i = 0;

        ASSERT (tok == INPUT, "INPUT expected");

        tok = gettok ();
        while (tok == IDENT) {
           set_input (definition, tokname);
           tok = gettok ();
           RETURN_WHEN (tok == RPAR);
           ERROR (tok != COMMA,
                        (stderr, "missing comma (%s)", nonterm_of (definition)));
           tok = gettok ();
        }

        ERROR (tok != OUTPUT, 
                   (stderr, "^ missing in %s", nonterm_of (definition)));
        return;
}

void	process_outtypes (definition)
register struct att_def *definition;
{
	ASSERT (tok == OUTPUT, "OUTPUT expected");

        tok = gettok ();
        while (tok == IDENT) {
           set_output (definition, tokname);
           tok = gettok ();
           RETURN_WHEN (tok == RPAR);
           ERROR (tok != COMMA,
                      (stderr, "Comma expected (%s)", nonterm_of (definition)));
           tok = gettok ();
        }

        ERROR (TRUE,
                 (stderr, "Identifier expected (%s)", nonterm_of (definition)));
        return;
}

/*
 *	Now for the rule recognition. Recall that
 *	the symbols of the rules still have to be
 *	processed by YACC or bison
 */

struct att_def *lhs_side ()
{
	register int	par_i_cnt	= 0;
	register int	par_o_cnt	= 0;

	register struct att_def *lhs_member;

	ERROR (tok != IDENT, (stderr,  "Syntax error in lhs, line %d", lineno));

        fprintf (foutput, "%s   :", tokname);
        lhs_member = lookup (tokname);

	tok = gettok ();
        if (tok == LPAR) {
           ERROR (lhs_member == (struct att_def *) 0, 
                        (stderr, "Illegal lhs %s", tokname));

           tok = gettok ();
           if (tok == INPUT) {
              tok = gettok ();
              while (tok == IDENT) {
                 par_i_cnt ++;
                 push (lhs_member, par_i_cnt, tokname);
                 tok = gettok ();
                 if (tok == RPAR) { /* the end of the road */
                    tok = gettok ();
                    if (par_i_cnt != in_pars (lhs_member) ||
                        par_o_cnt != out_pars (lhs_member) )
                       ERROR (TRUE, (stderr, "Incorrect number of params %s",
                                         nonterm_of (lhs_member) ));
                    return lhs_member;
                 }
                 ERROR (tok != COMMA, 
                           (stderr, "Comma expected (%s)", nonterm_of (lhs_member)));
                 tok = gettok ();
              }
           }

           if (tok == OUTPUT) {
              tok = gettok ();
              while (tok == IDENT) {
                 par_o_cnt ++;
                 dest_par (lhs_member, par_o_cnt, tokname);
                 tok = gettok ();
                 if (tok == RPAR) {
                    tok = gettok ();
                    if (par_i_cnt != in_pars (lhs_member) ||
                        par_o_cnt != out_pars (lhs_member) )
                       ERROR (TRUE, (stderr, "Incorrect number of params %s",
                                         nonterm_of (lhs_member) ));
                    return lhs_member;
                 }
                 
                 ERROR (tok != COMMA, (stderr,"Comma expected (%s)", nonterm_of (lhs_member)));
                 tok = gettok ();
              }
           }

	   ERROR (TRUE, 
                    (stderr, ") expected (%s)", nonterm_of (lhs_member)));
      }
}

void	rule ()
{
	register struct att_def *lhs_def;

	init_stack ();
	lhs_def = lhs_side ();

	ERROR (tok != COLON, 
                  (stderr, ": expected (%s)", nonterm_of (lhs_def)));

        tok = gettok ();

        process_alternative (lhs_def);
        while (tok == BAR) {
           fprintf (foutput, "\n	|");
	   re_init_stack ();
	   tok = gettok ();
           process_alternative (lhs_def);
        }

	ERROR (tok != SEMI,
                   (stderr, "; expected (%s)", nonterm_of (lhs_def)));

        fprintf (foutput, "\n");
}

void	process_alternative (lefthand)
register struct att_def *lefthand;
{
	while (tok == IDENT) {
           fprintf (foutput, "%s ", tokname);
           process_member (lefthand);
        }

        if (tok == EQ_SYMB) {
           fprintf (foutput, "=\n");
           tok = gettok ();
           cpyact ();
           tok = gettok ();
        }
        else
           adjust_stack ();
}

void	process_member (lefthand)
register struct att_def * lefthand;
{
	register int	in_index, out_index;
	register struct att_def *curr_member;

	ASSERT (tok == IDENT, "Token should be IDENT");

        curr_member = lookup (tokname);

        tok = gettok ();
        if (tok != LPAR) {
           if (curr_member == (struct att_def *)0)
              return;

           ERROR (in_pars (curr_member) + out_pars (curr_member) != 0,
                   (stderr, "Incorrect number of params (%s)", nonterm_of (lefthand) ));
           return;
        }

        if (curr_member == (struct att_def *)0) /* and tok == '(' */
           ERROR (TRUE, (stderr, "Parameters illegally supplied (%s)",
                          nonterm_of (lefthand) ));

        tok = gettok ();
        in_index	= 0;
	out_index	= 0;

	if (tok == INPUT) {
	   tok = gettok ();
           while (tok == IDENT) {
	      in_index ++;
	      verify_in (curr_member, in_index, tokname);
	      tok = gettok ();
	      if (tok == RPAR) {
                 ERROR (in_pars (curr_member) != in_index,
                        (stderr, "Incorrect number of params (%s)", nonterm_of (lefthand) ));
                 tok = gettok ();
                 return;
              }
            
              ERROR (tok != COMMA, 
                     (stderr, "Comma expected at member (%s)", nonterm_of (curr_member) ));

              tok = gettok ();
           }
        }

        if (tok == OUTPUT) {
           tok = gettok ();
           while (tok == IDENT) {
              out_index ++;
              verify_out (curr_member, out_index, tokname);
              tok = gettok ();
     
	      if (tok == RPAR) {
                 ERROR (out_pars (curr_member) != out_index,
                        (stderr, "Incorrect number of params (%s)", nonterm_of (lefthand) ));
                 tok = gettok ();
                 return;
              }
            
              ERROR (tok != COMMA, 
                     (stderr, "Comma expected at member (%s)", nonterm_of (curr_member) ));

              tok = gettok ();
           }
        }

        ERROR (TRUE, (stderr, "Either input or output expected (%s)", nonterm_of (curr_member)));
}


