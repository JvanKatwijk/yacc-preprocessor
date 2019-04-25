#
/*
 * Prep.h
 *
 * A simple preprocessor for YACC
 *
 * For a functional description: see Sigplan Notices 
 * October 1983, 18 (10) pp 12 .. 16
 */
//
//	added to get the thing compiled after 35 years

#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
//
//	end of added
#define	TRUE		01
#define	FALSE		00

#define MAX_ATTR	10
#define	PAR_SIZE	10
#define	TAG_SIZE	3000
#define MAXDEF		20
#define	NONTERM		500	
#define	WSTACKSIZE	40	/* arbitrary choice */
#define	OUTSTACKSIZE	20

#define	NAMESIZE	200

#define	FATAL		030
#define	WARNING		040

#define	AS_LOCAL	010
#define	AS_STACK	020
#define	LOC_STRING	"_L%dL_"

struct type_elem {
	int	un_number;
	char	*type_name;
};

struct param {
	int	par_no;
	int	direction;	/* INPUT or OUTPUT */
	struct type_elem *par_desc;
};

struct att_def {
	int	i_par,
		o_par;
	struct	param attributes [MAX_ATTR];
	char	*nonterm;
	int	hasdefinition;
};

#define	i_par_of(x) (x) -> i_par
#define o_par_of(x) (x) -> o_par
#define attributes_of(x) (x) -> attributes
#define nonterm_of(x) (x) -> nonterm

struct stack_elem {
	char	par_name [PAR_SIZE];
	struct	type_elem *par_def;
	struct {
		char	how_to_access;
		int	ac_offset;
		int	ac_lab;
	} access;		/* for access function */
};

/*
 *	Token definitions
 */
#define	ENDFILE		0
#define	COLON		1
#define	SEMI		2
#define	BAR		3
#define	INPUT		3	/* no mistake */
#define	EQ_SYMB		4
#define	IDENT		5
#define	TERM		6
#define	LEFT		7
#define	BINARY		8
#define	RIGHT		9
#define	MARK		10
#define	PREC		11
#define	LCURL		12
#define	START		13
#define	TYPE		14
#define	UNION		15
#define	ATTR_DEF	16
#define	COMMA		17
#define	LPAR		18
#define	RPAR		19
#define	OUTPUT		20
#define	NUMBER		21


extern	FILE	* finput,
                * foutput;
extern	int	lineno;
extern	int	tok;
extern	char	tokname[];
extern	char	*infile;

void	process_intypes ();
void	process_outtypes ();
struct att_def *lhs_side ();
void	rule ();
void	process_alternative ();
void	process_member ();
void	proc_def ();
void	process_intypes ();
void	process_outtypes ();
struct att_def *lhs_side ();
void	rule ();
void	process_alternative ();
void	process_member ();
int	is_namelem ();
int	is_control ();
int	gettok ();
void	cpyunion () ;
void	cpycode ()	;
int	skipcom ();
void	cpyact ();
char	*new_tag ();
void	print_union () ;
struct type_elem *add_type ();
struct att_def *lookup ();
struct att_def *new_def ();
void	set_input ();
void	set_output ();
int	in_pars ();
int	out_pars ();
void	init_stack ();
void	re_init_stack ();
void	cp_tag ();
struct type_elem *get_i_type ();
struct type_elem *get_o_type ();
void	push ();
void	dest_par ();
void	verify_in ();
void	verify_out ();
struct stack_elem *search_stack ();
void	pref_stackadjust ();
void	post_adjuststack ();
void	adjust_stack ();

//	added to get the thing compiled after well over 30 years
char	*address_of (char *);
//	end of added
#define	ASSERT(x, y) if (!(x)) {printf (y); return;}

#define RETURN_WHEN(x) if (x) return;

#define ERROR(x, z) if (x) { fprintf z; exit (1); }
