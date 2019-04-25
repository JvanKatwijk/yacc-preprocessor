#
 #	Makfile for YACC/BISON preprocessor
#
CFLAGS = -c -O
DEBUG  =-ggdb

prep:	prep.h prep1.o prep2.o prep3.o prep4.o
	cc  prep4.o prep[123].o
	mv a.out prep
	size prep

install:	prep
	-rm /usr/bin/prep
	ln prep /usr/bin/prep

prep1.o:	prep.h prep1.c
	gcc  -c $(CFLAGS) $(DEBUG) prep1.c

prep2.o:	prep.h prep2.c
	gcc  -c $(CFLAGS) $(DEBUG) prep2.c

prep3.o:	prep.h prep3.c
	gcc  -c $(CFLAGS) $(DEBUG) prep3.c

prep4.o:	prep.h prep4.c
	gcc  -c $(CFLAGS) $(DEBUG) prep4.c

