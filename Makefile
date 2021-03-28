# Dependency rules for non-file targets
all: testsymtablelist testsymtablehash
clobber: clean
	rm -f *~ \#*\#
clean:
	rm -f testsymtablelist testsymtablehash *.o

# Dependency rules for file targets
testsymtablelist: symtablelist.o testsymtable.o
	gcc217 symtablelist.o testsymtable.o -o testsymtablelist
symtablelist.o: symtablelist.c symtable.h
	gcc217 -c symtablelist.c
testsymtable.o: testsymtable.c symtable.h
	gcc217 -c testsymtable.c

testsymtablehash: symtablehash.o testsymtable.o
	gcc217 symtablehash.o testsymtable.o -o testsymtablehash
symtablehash.o: symtablehash.c symtable.h
	gcc217 -c symtablehash.c
testsymtable.o: testsymtable.c symtable.h
	gcc217 -c testsymtable.c
