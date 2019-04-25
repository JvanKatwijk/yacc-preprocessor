	/* @(#)y6.c	1.6	86/09/16 */
#include	<stdio.h>
#include	"prep.h"

/*
 *
 *	J van Katwijk
 *	Delft University of Technology,
 *	Department of Mathematics,
 *	132 Julianalaan
 *	Delft, The Netherlands
 *
 *	See for a functional description:
 *
 *	SIGPLAN NOTICES
 *	October 1983, 18 (10) pp 12 .. 16
 */
/*
 *	low level data/stack management routines for
 *	YACC preprocessor.
 *
 */

/*
 *	tags:
 */
static char name_array [TAG_SIZE];
static char *tag_base = &name_array [0];


static int eq_tags (x, y)
register char *x, *y;
{
	while (*x == *y && *x != (char)0 && *y != (char)0)
	      { x++;
	        y++;
	      }

	return (*x == *y);
}

int	is_similar (x, y)
register struct stack_elem *x, *y;
{
#ifdef DEBUG
	printf ("in is_sim %s %s\n", x -> par_name, y -> par_name);
#endif 
	if (eq_tags (x -> par_name, y -> par_name))
	   return x -> par_def -> type_name == y -> par_def -> type_name;
	else
	   return FALSE;
}

char *new_tag (t)
register char *t;
{
	register char *l_base = tag_base;

	while ( (*tag_base = *t) != (char)0)
	{ tag_base ++;
	  t++;
	}

	tag_base ++;	/* set for next definition */
	return l_base;
}

/*
 *	dealing with types and unions
 */

struct {
	int el_cnt;
	struct type_elem descr [MAXDEF];
} type_descr;

/*
 *	somewhere a definition of a union of all
 *	attribute element types is generated:
 */

void	print_union ()
{
	int i;
	struct type_elem *x;

	if (type_descr. el_cnt == 0) return;	/* nothing */

	fprintf (foutput, "\n#ifndef ATT_SIZE\n");
	fprintf (foutput, "#define ATT_SIZE	250\n");
	fprintf (foutput, "#endif\n");
	fprintf (foutput, "\nunion _attr_elem {\n");

	for (i = 0; i < type_descr. el_cnt; i++)
	{   x = &type_descr.descr [i];
	    fprintf (foutput, "  %s  _t%d;\n",
				x -> type_name, x -> un_number);
	}

	fprintf (foutput, "} _attr_stack [ATT_SIZE];\n");
	fprintf (foutput, "union _attr_elem *_ap =");
	fprintf (foutput, "      &_attr_stack [ATT_SIZE - 1];\n");
}

struct type_elem *add_type (t)
char *t;
{
	int i;
	struct type_elem *x;

	for (i = 0; i < type_descr . el_cnt; i++) {
           x = &type_descr . descr [i];
	   if (eq_tags (t, x -> type_name))
	       return x;
        }

	ERROR ((type_descr.el_cnt >= MAXDEF),
                 (stderr, "too many attribute defs: line %d\n", lineno));

	x = &type_descr. descr [type_descr. el_cnt];
	x -> type_name = new_tag (t);
	x -> un_number = type_descr. el_cnt ++;

	return x;
}


/*
 *	attribute definitions
 */
struct att_def attrib_defs [NONTERM];

static int ndefs = 0;


/*
 *	functions to be defined:
 *
 *		set_input	enter input attrib in def
 *		set_output	enter output attrib in def
 *		in_pars		number of inpars
 *		out_pars	number of outpars
 *		new_def		enter new non terminal
 *		lookup		look for nonterm.
 *		checkdefs	check all attributed symbols to be on lhs
 *		setdefined	set non terminal defined as lhs
 */

void	setdefined (t)
struct att_def *t;
{
	if (t != (struct att_def *)0) {
#ifdef DEBUG
	   printf ("defining %s\n", t -> nonterm);
#endif
	   t -> hasdefinition = TRUE;
	}
}

void	checkdefs ()
{
	register int i;

	for (i = 0; i < ndefs; i ++) {
		if (attrib_defs [i]. hasdefinition == (char) 0) {
			fprintf (stderr, "%s not as lhs", attrib_defs [i]. nonterm);
		}
	}
}

struct att_def *lookup (t)
register char *t;
{
	register int i;

	for (i = 0; i < ndefs; i++)
	    if (eq_tags (t, attrib_defs [i]. nonterm))
	       return &attrib_defs [i];

	return (struct att_def *)0;
}

struct att_def *new_def (t)
register char *t;
{
	register struct att_def *x;

	ERROR ((lookup (t) != (struct att_def *)0),
	   (stderr, "double definition line %d\n", lineno));

	ERROR ((ndefs >= NONTERM),
	   (stderr, "too much non terminals line %d\n", lineno));

	x = &attrib_defs [ndefs ++];
	x -> i_par = 0;
	x -> o_par = 0;
	x -> nonterm = new_tag (t);
	x -> hasdefinition = 0;

	return x;
}

void	set_input (def, typename)
register struct att_def *def;
register char *typename;
{
	register struct param *y;

	ERROR ( (def -> i_par >= MAX_ATTR),
	     (stderr, "too many input attribs line %d\n", lineno));

	def -> i_par++;
	y = &def -> attributes [def -> i_par];

	y -> par_no = def -> i_par;
	y -> direction = INPUT;
	y -> par_desc = add_type (typename);
}

void	set_output (def, type_name)
register struct att_def *def;
register char *type_name;
{
	register struct param *y;

	ERROR ( (def -> i_par + def -> o_par >= MAX_ATTR),
	   (stderr, "too many input/output attr's line %d\n", lineno));

	def -> o_par++;

	y = &def -> attributes [def -> i_par + def -> o_par];
	y -> par_no = def -> o_par;
	y -> direction = OUTPUT;
	y -> par_desc = add_type (type_name);
}

int	in_pars (def)
register struct att_def *def;
{
	if (def == (struct att_def *)0) return 0;
	return def -> i_par;
}

int	out_pars (def)
register struct att_def *def;
{
	if (def == (struct att_def *)0) return 0;
	return def -> o_par;
}

/*
 *	during the processing of a rule a stack is maintained
 *	for the simulation of the run time behaviour
 */
struct {
	int i_top;		/* input pars lhs	*/
	int o_top;		/* output pars lhs	*/
	int w_top;		/* workstack top	*/

	struct stack_elem work_stack [WSTACKSIZE];
	struct stack_elem out_stack  [OUTSTACKSIZE];
} workstack;

#ifdef PRINT
pr_stack ()
{
	register int i;

	for (i = workstack. w_top; i > 0; i--)
	    printf (" ap [%d] = %s\n", i + workstack. o_top,
				workstack. work_stack [i]. par_name);

	printf ("output stack\n");
	for (i = workstack. o_top; i > 0; i --)
	    printf (" %s\n", workstack. out_stack [i]. par_name);
}
#endif 
/*
 *
 *	functions available during processing a rule:
 *
 *
 *	init_stack ();
 *
 *	push (def, cnt, tag);	/* lhs input param
 *	dest_par (def, cnt, tag)	/* lhs output param
 *
 *	veri_in (member, parno, tag);
 *	veri_out(member, parnp, tag);
 *
 *	address_of (tag)	/* map into a string used for C 
 *
 *	pref_stackadjust ();	/* code for stack adjustment 
 *	post_stackadjust ();	/* code for stack adjustment 
 */
void	init_stack ()
{
	workstack. i_top = 0;
	workstack. o_top = 0;
	workstack. w_top = 0;
}

void	re_init_stack ()
{
	workstack. w_top = workstack. i_top;
}


void	cp_tag (x, y, cnt)
register char *x, *y;
register int cnt;
{
	while (--cnt >= 0)
	      *y++ = *x++;
}


struct type_elem *get_i_type (def, parno)
register struct att_def *def;
register int parno;
{
	ERROR ( (parno > def -> i_par),
	      (stderr, "too high input param for %s (line %d)" , def -> nonterm, lineno));

	return (def -> attributes [parno].par_desc);
}

struct type_elem *get_o_type (def, parno)
register struct att_def *def;
register int parno;
{
	ERROR ( (parno > def -> o_par),
	   (stderr, "too high output param number for %s (line %d)", def -> nonterm, lineno));

	return (def -> attributes [def -> i_par + parno]. par_desc);
}


void	push (def, parno, tag)
struct att_def *def;
char *tag;
int parno;
{
	struct stack_elem *x;

	ERROR ( (workstack. i_top ++ >= WSTACKSIZE),
	   (stderr, "too much attributes on workstack line %d\n", lineno));
	x = &workstack. work_stack [workstack. i_top];
	cp_tag (tag, &x -> par_name [0], PAR_SIZE);
	x -> par_def = get_i_type (def, parno);
	workstack. w_top = workstack. i_top;
#ifdef DEBUG
	printf ("push: %s %d %s\n",
				&x -> par_name [0], workstack. i_top,
						x -> par_def -> type_name);
#endif
}

void	dest_par (def, parno, tag)
register struct att_def *def;
register char *tag;
int parno;
{
	register struct stack_elem *x;

	ERROR ( (workstack.o_top ++ >= OUTSTACKSIZE),
	   (stderr, "too much lhs out attributes line %d\n", lineno));

	x = &workstack. out_stack [workstack. o_top];
	cp_tag (tag, &x -> par_name [0], PAR_SIZE);
	x -> par_def = get_o_type (def, parno);
#ifdef DEBUG
	printf ("dest_par : %s %d %s\n",
			x -> par_name, workstack. o_top,
			x -> par_def -> type_name);
#endif
}

void	verify_in (def, parno, tag)
struct att_def *def;
char *tag;
int parno;
{
	struct stack_elem *x;

	x = &workstack. work_stack[ workstack. w_top
					- in_pars (def) + parno];
	ERROR ( (!eq_tags (x -> par_name, tag)),
	   (stderr, "non matching input name %s (line %d)", tag, lineno));
#ifdef DEBUG
	printf ("veri_in: %s at offset %d\n", tag, 
				workstack. w_top - in_pars (def) + parno);
#endif
}

void	verify_out (def, parno, tag)
struct att_def *def;
char *tag;
int parno;
{
	struct stack_elem *x;

	ERROR ( (workstack. w_top++ >= WSTACKSIZE),
	   (stderr, "too much attribute values line %d\n", lineno));

	x = &workstack. work_stack [workstack. w_top];
	cp_tag (tag, &x -> par_name [0], PAR_SIZE);
	x -> par_def = get_o_type (def, parno);
#ifdef DEBUG
	printf ("veri_out: %s %d %s\n",
		     x -> par_name, workstack. w_top,
					x -> par_def -> type_name);
#endif
}

/*
 * given the tag of an attribute, translate it into a string
 * containing the 'C' code to adress the element on the
 * attribute stack (or the local....)
 */
struct stack_elem *search_stck (t)
register char *t;
{
	register int i;
	register struct stack_elem *x;

	for (i=1; i <= workstack. o_top; i++)
	    { x = &workstack. out_stack [i];
#ifdef DEBUG
		printf ("search: %s\n", &x -> par_name [0]);
#endif
	      if (eq_tags (t, x -> par_name))
	         return x;
	    }
/*
 *	now on the input stack
 */
	for (i = workstack. w_top; i > 0; i--)
	    { x = &workstack. work_stack [i];
#ifdef DEBUG
		printf ("search: %s\n", x -> par_name);
#endif
	      if (eq_tags (t, x -> par_name))
	         return x;
	    }

	for (i = workstack. w_top; i > 0; i--)
	    printf (" ap [%d] = %s\n", i + workstack. o_top,
				workstack. work_stack [i]. par_name);

	for (i = workstack. o_top; i > 0; i --)
	    printf (" %s\n", workstack. out_stack [i]. par_name);
	ERROR (TRUE,
                (stderr, "definition of attribute %s not found line (%d)", t, lineno));
}

char	*address_of (tag)
char	*tag;
{
	struct stack_elem *t;
	static char s [100];	/* most numbers will do */

	t = search_stck (tag);
	if (t -> access.how_to_access == AS_LOCAL)
	   sprintf (s, LOC_STRING, t -> access. ac_lab);
	else
	sprintf (s, "_ap [%d]. _t%d",
			t -> access. ac_offset, t -> par_def -> un_number);

	return s;
}


/*
 *	generate code for stack adjustment and to make
 *	output parameters addressable
 */
void	pref_stackadjust ()
{
	struct stack_elem *x, *y;
	int i;
	int l_top = 0;

#ifdef PRINT
	printf ("entered pref_stackadjust\n");
	pr_stack ();
#endif

	fprintf (foutput, "{");
	if (workstack. i_top == workstack. w_top)
	   { /* no intermediate results, lengthen
	      * the stack
	      */
	     if (workstack. o_top != 0)
	        fprintf (foutput, "	_ap -= %d;\n",
					workstack. o_top);
	     l_top = workstack. o_top;

	/* input parameter addressable below output parameters	*/

	     for (i = workstack. w_top; i > 0; i--)
	         { x = &workstack. work_stack [i];
	           x -> access.how_to_access = AS_STACK;
	           x -> access.ac_offset = 
				     l_top + workstack. w_top - i;
	         }
	     for (i = workstack. o_top; i >= 1; i--)
	         { x = &workstack. out_stack [i];
		   x -> access.how_to_access = AS_STACK;
		   x -> access. ac_offset = workstack. o_top - i;
	         }
	   }
	else
	{
	/* generate for each output parameter a local
	 * UNLESS it is named and types similar to the
	 * workstack attributes
	 */
	   for (i = workstack. w_top; i > 0; i--)
	       { x = &workstack. work_stack [i];
	         x -> access.how_to_access = AS_STACK;
	         x -> access.ac_offset = 
				   l_top + workstack. w_top - i;
	       }

	   for (i = 1; i <= workstack. o_top; i++)
	       { x = &workstack. out_stack [i];
	         y = &workstack. work_stack [workstack. i_top + i];

	         if (i <= workstack. w_top - workstack. i_top &&
					is_similar (x, y))
	            { x -> access. how_to_access = AS_STACK;
	   	      x -> access. ac_offset = y -> access. ac_offset;
		    }

	            else
	            { x -> access. how_to_access = AS_LOCAL;
	              x -> access. ac_lab = i;

		      fprintf (foutput, " %s ", x -> par_def -> type_name);
	   	      fprintf (foutput, LOC_STRING, i);
	   	      fprintf (foutput, ";\n");
	            }
	        }
	}
}

/*
 *	   after copying the C actions, we possibly have to adjust
 *	   the attribute value stack
 */
void	post_adjuststack ()
{
	   struct stack_elem *x;
	   int i;

	   if (workstack. i_top == workstack. w_top)
	      { /* adjustment already doen, no pushing/popping */
	         fprintf(foutput, "}\n");
	      }
	   else
	   { /* generate code for adjusting ap */
	     int stack_diff = workstack. w_top -
	   			workstack. i_top - workstack. o_top;

	     if (stack_diff != 0)
	        fprintf (foutput,"\n	_ap += %d;\n", stack_diff);
	     else
	        fprintf (foutput, "\n");

	     for (i = 1; i <= workstack. o_top; i++)
	         { x = &workstack. out_stack [i];
	           if (x -> access. how_to_access == AS_LOCAL)
	   	   { fprintf (foutput, "_ap [%d]. _t%d =",
/* do not try to understand the following computation	   */
/* I am a little ashamed of putting it down this way	   */
	   	                   workstack. o_top - x -> access. ac_lab,
	   	                   x -> par_def -> un_number);
	   	     fprintf (foutput, LOC_STRING,
	   			                x -> access. ac_lab);
	   	     fprintf (foutput, ";\n");
	              }
	         }
	      fprintf (foutput, "	}\n");
	}
}

/*
 *	no yacc actions specified, verify the work/output stack,
 *	adjust if necessary
 */
void	adjust_stack ()
{
	register int diff;
	register int i;

	diff = workstack. w_top - workstack. i_top - workstack. o_top;

	if (diff != 0) {
                fprintf (foutput, " {\n");
                fprintf (foutput, "_ap += %d; }", diff);
	}

	if (diff < 0) {
		fprintf (stderr, "/*garbage on extended attribute stack*/");
		return;
	}

	for (i = 1; i <= workstack. o_top; i++)
	    {
		if (workstack.out_stack [workstack. o_top - i + 1].par_def !=
		    workstack.work_stack [workstack. w_top -i + 1].par_def) {
			fprintf (stderr, "unequal default transfers %s",
					workstack. out_stack [i -1]. par_name);
                  }
	    }
}

