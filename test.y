
%term XUNION
%term XRECORD
%term SEMCOL
%term IDENT
%term STARREF
%term COMMA
%term LBRACK
%term RBRACK
%term SUB
%term BUS
%term NUMBER
%%
tree   :init record_list ;

init   :
{{ init_fields ();}
};

record_list   :
	|record_list record_desc  {
_ap += 1; }
	|record_list union_desc  {
_ap += 1; };

record_desc   :record_start rec_fields RBRACK opt_ident SEMCOL 
{{ printf ("/* end of record %s*/ \n", gettag (_ap [0]. _t0));
		  pop_untab (_ap [0]. _t0);
		
	}
};

union_desc   :union_start rec_fields RBRACK opt_ident SEMCOL 
{{ printf ("\n /* end of union %s */ \n", gettag (_ap [0]. _t0));
		  pop_untab (_ap [0]. _t0);
		
	}
};

opt_ident   :IDENT 
	|
{{ $$ = NULL; }
};

record_start   :XRECORD IDENT LBRACK 
{	_ap -= 1;
{ _ap [0]. _t0 = new_untab ($2, XRECORD);
		}
};

union_start   :XUNION IDENT LBRACK 
{	_ap -= 1;
{ _ap [0]. _t0 = new_untab ($2, XUNION);
		}
};

rec_fields   :rec_field 
	|rec_fields rec_field ;

rec_field   :field_type id_list SEMCOL 
{{ cfree (_ap [0]. _t1);
		
	_ap += 1;
	}
}
	|record_start rec_fields RBRACK opt_ident SEMCOL 
{{ if ($4 != NULL && does_exist ($4)) {
		     warning ("possible error with %s\n", $4);
		  }
		  pop_untab (_ap [0]. _t0);
		
	_ap += 1;
	}
};

id_list   :IDENT 
{{ if (!does_exist ($1)) {
		     pr_access (_ap [1]. _t0, _ap [0]. _t1, $1);
		     add_field ($1);
		  }
		}
}
	|id_list COMMA IDENT 
{{ if (!does_exist ($3)) {
		     pr_access (_ap [1]. _t0, _ap [0]. _t1, $3);
		     add_field ($3);
		  }
		}
};

field_type   :IDENT 
{	_ap -= 1;
{ _ap [0]. _t1 = new_string ($1);
		}
}
	|field_type STARREF 
{{ _ap [0]. _t1 = strcat (_ap [0]. _t1, " *");
		
	}
}
	|field_type SUB NUMBER BUS 
{{ _ap [0]. _t1 = strcat (_ap [0]. _t1, $3);
		
	}
};


%%

typedef	char	*pchar;

char	*mk_tag ();
int	line_no = 0;
#define	DEPTH	20
#define	NNAMES	600
#define	MAXTYPENAME	30
#define	FOR(x, y)	for (x = 0; x < y; x ++)

#include	<stdio.h>
#include	"lex.yy.c"

/*
 *	Stack of "open" record structures
 */
struct bla {
	char	*rectag;
	int	recflags;
};

struct recstack {
	struct bla X [DEPTH];
	int	top;
} _recstack;

struct {
	char	*names[NNAMES];
	int	filp;
} fieldnames;

init_fields ()
{
	_recstack. top = -1;
	fieldnames. filp = 0;
	printf ("\n");
}

new_untab (x, y)
char	*x;
int	y;
{
	if (_recstack. top >= DEPTH) {
	   warning ("Help, stack full");
	   exit (21);
	}

	_recstack. top ++;
	_recstack. X [_recstack. top]. rectag = x;
	_recstack. X [_recstack. top]. recflags = y;
	return _recstack. top;
}

pop_untab (x)
int x;
{

	if (_recstack. top != x) {
	   printf ("Synchronization error\n");
	   exit (22);
	}
	_recstack. top --;
}


int	does_exist (t)
register char *t;
{
	int i;
	FOR (i, fieldnames. filp) {
	    if (eq_tags (t, fieldnames. names [i]))
	       return (-1);
	}
	return 0;
}

add_field (t)
register char *t;
{
	fieldnames. filp ++;
	fieldnames. names [fieldnames. filp] = t;
}

char	*mk_tag (s, l)
register char *s;
register int l;
{
	register int d;
	register char *c = calloc (1, l);

	if (c == (char *)-1) {
	   printf ("Ran out of memory");
	   exit (23);
	}
	FOR (d, l + 1) {
	   c [d] = s [d];
	}

/*
	printf ("Added %s\n", c);
 */
	return c;
}

warning (s, p)
char	*s;
int	p;
{
	printf (s, p);
}

char	*new_string (s)
char	*s;
{
	char	*c = calloc (1, MAXTYPENAME);

	if (c == (char *)-1) {
	   printf ("Ran out of memory\n");
	   exit (24);
	}

	c [0] = '\0';
	return strcat (c, s);
}


sel_code (x)
int x;
{
	if (x < 0) {
	   printf ("Help, x < 0");
	   abort ();
	}

	if (x == 0) {
	   if (_recstack. X [0]. recflags == XRECORD)
	      printf ("(( *(struct %s *)x)", _recstack. X [_recstack. top]. rectag);
	   else
	      printf ("((*x)");
	   return;
	}

	sel_code (x - 1);
	printf (".%s", _recstack. X [x]);
}


pr_access (x, y, z)
int x;
char *y;
char *z;
{

	printf ("#define	g_%s(x)\t", z);
	sel_code (x);
	printf (".%s", z);
	printf (")\n");
	printf ("#define	s_%s(x, y)\t", z);
	sel_code (x);
	printf (".%s = ((%s)y))\n", z, y);
}
char	*gettag (x)
int x;
{
/*
	return _recstack. X [_recstack. top]. rectag;
 */
	return _recstack. X [0]. rectag;
}

int	eq_tags (x, y)
register char *x, *y;
{
	while (*x == *y) {
	   if (*x == '\0')
	      return -1;
	   if (*y == '\0')
	      return -1;
	   x ++;
	   y ++;
	}

	return 0;
}


main (argc, argv)
int	argc;
char	**argv;
{
	if (argc < 2) {
	   printf ("Usage %s: %s filename\n", argv [0], argv [0]);
	   exit (101);
	}

	yyin = fopen (argv [1], "r");
	if (yyin == (FILE *)-1) {
	   printf ("cannot open %s\n", argv [1]);
	   exit (102);
	}
	yyparse ();
}

yyerror ()
{
	printf ("Unrecoverable error (%d)\n", line_no);
	exit (25);
}


#ifndef ATT_SIZE
#define ATT_SIZE	250
#endif

union _attr_elem {
  int  _t0;
  pchar  _t1;
} _attr_stack [ATT_SIZE];
union _attr_elem *_ap =      &_attr_stack [ATT_SIZE - 1];

 /* processed by YACC PREP */ 
