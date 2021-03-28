/*--------------------------------------------------------------------*/
/* symtable.h                                                         */
/* Author: Julio Lins (jcclb)                                         */
/*--------------------------------------------------------------------*/

#ifndef SYMTABLE_H
#define SYMTABLE_H

/*--------------------------------------------------------------------*/

#include <stddef.h>

/*--------------------------------------------------------------------*/

/* A SymTable_T object is an unordered collection of key-value 
   bindings */

typedef struct SymTable *SymTable_T;

/*--------------------------------------------------------------------*/


/* Return a new SymTable_T object, or NULL if insufficient memory is
   available */

SymTable_T SymTable_new(void);

/*--------------------------------------------------------------------*/

/* Free memory allocated by oSymTable */

void SymTable_free(SymTable_T oSymTable);

/*--------------------------------------------------------------------*/

/* Return length of oSymTable (i.e., number of bindings) */

size_t SymTable_getLength(SymTable_T oSymTable);

/*--------------------------------------------------------------------*/

/* Add a binding with pcKey as key and pvValue as value to oSymTable.
   Return 1 (TRUE) if insertion is successful, and 0 (FALSE) if there 
   is insufficient memory or if there is already a binding whose key is
   pcKey, in which case oSymTable is left unchanged */

int SymTable_put(SymTable_T oSymTable, const char *pcKey,
                 const void *pvValue);

/*--------------------------------------------------------------------*/

/* If oSymTable contains a binding whose key is pcKey, assign pvValue
   as the binding's new value and return its previous value. Else, 
   leave oSymTable unchanged and return NULL */

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue);

/*--------------------------------------------------------------------*/

/* Return 1 (TRUE) if oSymTable contains a binding whose key is pcKey,
   0 (FALSE) otherwise */

int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* Return the value of the binding within oSymTable whose key is pcKey,
   or NULL if no such binding exists */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* If oSymTable contains a binding whose key is pcKey, remove such
   binding and return its value. Else, leave oSymTable unchanged and
   return NULL */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* Applies function pfApply to each binding in oSymTable, passing each
   bindings' key (pcKey) and value (pvValue) as parameters, as well as
   pvExtra as en extra parameter */

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue,
                                  void *pvExtra),const void *pvExtra);

/*--------------------------------------------------------------------*/

#endif
