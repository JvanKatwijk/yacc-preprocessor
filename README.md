
A simple preprocessor for YACC

-------------------------------------------------------------------------
Introduction
-------------------------------------------------------------------------

In the early 80-ies we - at TU Delft - were developing an ADA compiler,
written in C. The project started on a PDP-11, running PWB-UNIX,
it continued on an M68000 based machine, built by TNO (the "Geminix").

The syntax for ADA is pretty large, and at the time we choose to
use YACC as parser generator for handling the syntax (interesting to see
that the Gnat parser is a handwritten top-down parser)

The ADA syntax was written in well over 800 rules, and it was only a
rough coverage (remember the A(i), that could have different meanings).

To handle a basic flow of parameters in the processing of
the  syntax rules, used to do some basic checking but mostly to
decorate a parse tree, I created the possibility of adding more
parameters than the
single $1, $2, ... $$ as provided by YACC,
i.e. a simple attributing mechanism. For that purpose
a simple preprocessor was built, the sources are in this repository.

Of course, evaluation is in a single pass, and parameter values should
be known when used, so there are limitations.

The software was described in the October 1983 issue
of SIGPLAN notices, 18 (10) pp 12 .. 16.

Anyway, it helped and we built a pretty decent Ada compiler
used on the afore mentioned Geminix computer,
the development of which, by lack of
funding and interest, went the way of many interesting projects and stopped.

Btw, during the development of the Ada compiler, we had as a student project
extending the YACC version we used by including the preprocessing and inclusing
a pretty decent error recovery mechanism.
It seems that the sources of this "extended YACC" are lost.

I discovered the sources of the preprocessor -they were stored on floppy discs -
and - after 35 years - it took only 3 or 4 minor changes to compile
the sources and build another "prep".

------------------------------------------------------------------------
An example
-------------------------------------------------------------------------

An example of a rule
	
	rec_field (|x): field_type (|x, ^t) id_list (|x, t) SEMCOL =
                { cfree ($t);
                } |
                record_start (^x1) rec_fields (|x1)
                             RBRACK opt_ident SEMCOL =
                { if ($4 != NULL && does_exist ($4)) {
                     warning ("possible error with %s\n", $4);
                  }
                  pop_untab ($x1);
                };

This shows that the rec_field has an inherited attribute, field type
has as first attribute the attrbute inherited from rec_field and delivers
a derived attribute, while these two attributes are passed on to id_list
as inherited attributes. 

This rule will then be translated into

	rec_field   :field_type id_list SEMCOL
	{{ cfree (_ap [0]. _t1);^M
        _ap += 1;
        	}
	}
        |record_start rec_fields RBRACK opt_ident SEMCOL
	{{ if ($4 != NULL && does_exist ($4)) {^M
                     warning ("possible error with %s\n", $4);^M
                  }^M
                  pop_untab (_ap [0]. _t0);^M

        	_ap += 1;
        	}
	};


---------------------------------------------------------------------------
Creating and use
---------------------------------------------------------------------------

Assuming a C compiler is installed, creating an executable is by
	
	make

Use of the preprocessor is
	prep outputfile [inputfile]




----------------------------------------------------------------------------
Copyright
----------------------------------------------------------------------------

# Copyright


        Copyright (C)  1982 .. 2019
        Jan van Katwijk (J.vanKatwijk@gmail.com)
        Lazy Chair Computing

