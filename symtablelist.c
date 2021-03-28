/*____________________________________________________________________*/
/* symtablelist.c                                                     */
/* author: Julio Lins (jcclb)                                         */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/


/* Each key and respective value are stored in a Node. Nodes are linked
   to form a list */

struct Node {

   /* key, owned by implementation through defensive copy */
   const char *pcKey;

   /* value, owned by client */
   void *pvValue;

   /* The address of the next Node */
   struct Node *pnNext;
};


/* SymTable is a structure that points to the first Node */

struct SymTable {

   /* Address of the list's first Node */
   struct Node *pnFirst;

   /* size of Symble Table (total # of Nodes) */
   size_t uLength;
};


SymTable_T SymTable_new(void) {

   SymTable_T oSymTable;

   oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

   if (oSymTable == NULL)
      return NULL;
   
   oSymTable->pnFirst = NULL;
   oSymTable->uLength = 0;
   
   return oSymTable;
}


void SymTable_free(SymTable_T oSymTable) {

   struct Node *pnNext;
   struct Node *pnCurrent;

   
   assert(oSymTable != NULL);

   
   pnCurrent = oSymTable->pnFirst;

   
   while (pnCurrent != NULL) {

      /* Save pointer to next Node before freeing pnCurrent */
      pnNext = pnCurrent->pnNext;

      
      free((void*)pnCurrent->pcKey);
      
      free(pnCurrent);

      pnCurrent = pnNext;
   }
   
   free(oSymTable);
}


size_t SymTable_getLength(SymTable_T oSymTable) {

   assert(oSymTable != NULL);
   
   return oSymTable->uLength;
}


int SymTable_put(SymTable_T oSymTable, const char *pcKey,
                 const void *pvValue) {

   struct Node *pnNewNode;
   struct Node *pnCurrent;
   char *pcCopy;
   enum {FALSE, TRUE};
   enum {EQUAL};

   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   
   pnCurrent = oSymTable->pnFirst;
   
   
   while(pnCurrent != NULL) {

      /* if key is already stored, do not put it again */
      if (strcmp(pnCurrent->pcKey, pcKey) == EQUAL)
         return FALSE;

      pnCurrent = pnCurrent->pnNext;
   }

   
   pnNewNode = (struct Node*)malloc(sizeof(struct Node));

   if (pnNewNode == NULL)
      return FALSE;

   /* Makes and assigns defensive copy */
   pcCopy = (char*)malloc(strlen(pcKey) + 1);
   
   if (pcCopy == NULL) {

      free(pnNewNode);
      
      return FALSE;
   }

   pcCopy = strcpy(pcCopy, pcKey);
   
   pnNewNode->pcKey = pcCopy;


   pnNewNode->pvValue = (void*)pvValue;


   pnNewNode->pnNext = oSymTable->pnFirst;
   oSymTable->pnFirst = pnNewNode;

   
   oSymTable->uLength++;


   return TRUE;
}

/* Return Node of corresponding key, if found. oSymTable is the Symble
   Table object of which pcKey might or might not be a key. If a search
   hit, it returns a pointer to pcKey's Node. Else, it returns NULL */

static struct Node *SymTable_find(SymTable_T oSymTable,
                                 const char *pcKey) {

   enum {EQUAL};
   struct Node *pnCurrent;


   /* Redundant, but just so that critTer doesn't complain */
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   pnCurrent = oSymTable->pnFirst;

   while (pnCurrent != NULL) {

      if (strcmp(pnCurrent->pcKey, pcKey) == EQUAL)
         return pnCurrent; 

      pnCurrent = pnCurrent->pnNext;
   }

   return NULL;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue) {

   struct Node *pnResult;
   void *pvPrevious;
   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   

   pnResult = SymTable_find(oSymTable, pcKey);

   if (pnResult == NULL) return NULL;

   pvPrevious = pnResult->pvValue;
   pnResult->pvValue = (void*)pvValue;

   return pvPrevious;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {

   enum {NOT_FOUND, FOUND};
   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   if (SymTable_find(oSymTable, pcKey) == NULL) return NOT_FOUND;

   return FOUND;
   
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {

   struct Node *pnResult;
   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   pnResult = SymTable_find(oSymTable, pcKey);

   if (pnResult == NULL) return NULL;

   return pnResult->pvValue;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {

   struct Node *pnPrev;
   struct Node *pnCurrent;
   void *pvValue;
   enum {EQUAL};
   
   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   
   pnCurrent = oSymTable->pnFirst;

   
   pnPrev = NULL;
   
   while (pnCurrent != NULL) {

      if (strcmp(pnCurrent->pcKey, pcKey) == EQUAL) {

         /* if Node is the first on the list */
         if (pnPrev == NULL)
            oSymTable->pnFirst = pnCurrent->pnNext;

         else
            pnPrev->pnNext = pnCurrent->pnNext;
         
         
         pvValue = pnCurrent->pvValue;

         
         free((void*)pnCurrent->pcKey);

         free(pnCurrent);

         oSymTable->uLength--;

         return pvValue;
      }

      pnPrev = pnCurrent;
      pnCurrent = pnCurrent->pnNext;
   }

   return NULL;
   
}

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue,
                                  void *pvExtra), const void *pvExtra) {

   struct Node *pnCurrent;
   
   assert(oSymTable != NULL);
   assert(pfApply != NULL);

   
   pnCurrent = oSymTable->pnFirst;

   
   while (pnCurrent != NULL) {

      (*pfApply)((void*)pnCurrent->pcKey,
                 (void*)pnCurrent->pvValue,
                 (void*)pvExtra);

      pnCurrent = pnCurrent->pnNext;
   }
}
