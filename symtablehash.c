/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* author: Julio Lins (jcclb)                                         */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/

/* Sequence of bucket counts for Symble Table expansion */

static const size_t auBucketCount[] = {509, 1021, 2039, 4093, 8191,
                                       16381, 32749, 65521};


/* Each key and respective value are stored in a Binding. Bindings
   whose keys hash to the same code are linked to form a list */

struct Binding {

   /* key, owned by implementation through defensive copy */
   const char *pcKey;

   /* value, owned by client */
   void *pvValue;

   /* The address of the next Binding on the list of same-hash-code
      Bindings */
   struct Binding *pbNext;
};


/* SymTable is a structure that points to all separate chains' first
   Bindings. That is, to all Bindings that are first on their list of
   same-hash-code Bindings */

struct SymTable {

   /* Index of auBucketCount[] leading to current bucket count for 
      SymTable object */
   size_t uBucketIndex;

   /* Pointer to the addresses of separate chains' first Bindings */
   struct Binding **ppbBuckets;

   /* size of Symble Table (total # of Bindings) */
   size_t uLength;
};



SymTable_T SymTable_new(void) {

   SymTable_T oSymTable;


   oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

   if (oSymTable == NULL)
      return NULL;


   oSymTable->ppbBuckets = (struct Binding**)
      calloc(sizeof(struct Binding*), auBucketCount[0]);

   if (oSymTable->ppbBuckets == NULL) {
      
      free(oSymTable);
      return NULL;
   }

   oSymTable->uLength = 0;
   oSymTable->uBucketIndex = 0;

   
   return oSymTable;
}



void SymTable_free(SymTable_T oSymTable) {

   struct Binding *pbCurrent;
   struct Binding *pbNext;
   size_t i;

   
   assert(oSymTable != NULL);

   
   for (i = 0; i < auBucketCount[oSymTable->uBucketIndex]; i++) {

      
      pbCurrent = oSymTable->ppbBuckets[i];    

      
      while (pbCurrent != NULL) {

         /* Save pointer to next Binding before freeing pbCurrent */
         pbNext = pbCurrent->pbNext;

         
         free((void*)pbCurrent->pcKey);

         free(pbCurrent);

         pbCurrent = pbNext;
      }
   }

   free(oSymTable->ppbBuckets);
   free(oSymTable);
}


size_t SymTable_getLength(SymTable_T oSymTable) {

   assert(oSymTable != NULL);

   return oSymTable->uLength;
}



/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. pcKey is a pointer to the key which will be hashed.
   uBucketCount is the current bucket count. */

static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}


/* Helper function that expands Symble Table to next bucket count.
   oSymTable is a pointer to the Symble Table that will be expanded. 
   If not enough memory for expansion, oSymTable does not change */

static void SymTable_grow(SymTable_T oSymTable) {

   size_t uCurrentIndex;
   size_t uNextIndex;
   size_t uNextCount;
   size_t i;
 
   struct Binding* pbCurrent;
   struct Binding** ppbTemp;

   /* oTempSymTable is a placeholder SymTable_T object. We will
      put all oSymTable Bindings into it, but with the new hash codes */
   
   SymTable_T oTempSymTable;


   uCurrentIndex = oSymTable->uBucketIndex; 
   uNextIndex = uCurrentIndex + 1;
   uNextCount = auBucketCount[uNextIndex];


   oTempSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
   

   if (oTempSymTable == NULL)
      return;

   
   oTempSymTable->ppbBuckets =
      (struct Binding**)calloc(sizeof(struct Binding*), uNextCount);

   if (oTempSymTable->ppbBuckets == NULL) {

      free(oTempSymTable);
      return;
   }

  
   oTempSymTable->uLength = 0;
   oTempSymTable->uBucketIndex = uNextIndex;

   /* Reinserts all key-value pairs into placeholder, but now hashing
      from 0 to uNextCount - 1 */

   for (i = 0; i < auBucketCount[uCurrentIndex]; i++) {
  
      pbCurrent = oSymTable->ppbBuckets[i];

      
      while (pbCurrent != NULL) {

         SymTable_put(oTempSymTable, pbCurrent->pcKey,
                      pbCurrent->pvValue);

         pbCurrent = pbCurrent->pbNext;

      }
   }

   /* Assigns expanded hash table to client's oSymTable. Frees
      both the old hash table and the placeholder oTempSymTable */

   ppbTemp = oSymTable->ppbBuckets;
   oSymTable->ppbBuckets = oTempSymTable->ppbBuckets;
   oTempSymTable->ppbBuckets = ppbTemp;


   oSymTable->uBucketIndex = uNextIndex;
   oTempSymTable->uBucketIndex = uCurrentIndex;
   
   SymTable_free(oTempSymTable);


}


int SymTable_put(SymTable_T oSymTable, const char *pcKey,
                 const void *pvValue) {

   size_t uMAX_INDEX;
   size_t uCurrentIndex;
   size_t uBucketCount;
   size_t uLength;
   

   size_t uIndex;
   char *pcCopy;
   struct Binding *pbCurrent;
   struct Binding *pbNewBinding;
   enum {FALSE, TRUE};
   enum {EQUAL};

   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   
   uMAX_INDEX = sizeof(auBucketCount)/sizeof(size_t) - 1;
   
   uCurrentIndex = oSymTable->uBucketIndex;
   
   uBucketCount = auBucketCount[uCurrentIndex];
   
   uLength = oSymTable->uLength;
   

   
   if ((uLength == uBucketCount) && (uCurrentIndex < uMAX_INDEX)) {
      
      SymTable_grow(oSymTable);
      
      uBucketCount = auBucketCount[oSymTable->uBucketIndex];
   }

   
   
   uIndex = SymTable_hash(pcKey, uBucketCount);
   
   
   pbCurrent = (oSymTable->ppbBuckets[uIndex]);
   
      
      while (pbCurrent != NULL) {
      
         if (strcmp(pbCurrent->pcKey, pcKey) == EQUAL)
            return FALSE;
      
         pbCurrent = pbCurrent->pbNext;
      }
   

   
   pbNewBinding = (struct Binding*)malloc(sizeof(struct Binding));

   if (pbNewBinding == NULL)
      return FALSE;

   
   pcCopy = (char*)malloc(strlen(pcKey) + 1);
   
   if (pcCopy == NULL) {

      free(pbNewBinding);

      return FALSE;
   }


   pcCopy = strcpy(pcCopy, pcKey);

   
   pbNewBinding->pcKey = pcCopy;
   
   pbNewBinding->pvValue = (void*)pvValue;



   pbNewBinding->pbNext = oSymTable->ppbBuckets[uIndex];

   oSymTable->ppbBuckets[uIndex] = pbNewBinding;
   
 
   oSymTable->uLength++;

   
   return TRUE;
}

/* Return Binding of corresponding key, if found. oSymTable is the
   Symble Table object of which pcKey might or might not be a key. If
   a search hit, it returns a pointer to pcKey's Binding. Else, it
   returns NULL */

static struct Binding *SymTable_find(SymTable_T oSymTable,
                                     const char *pcKey) {

   size_t uIndex;
   size_t uBucketCount;
   struct Binding *pbCurrent;
   enum {EQUAL};

   /* redundant, but just so that critTer doesn't complain */
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   
   
   uBucketCount = auBucketCount[oSymTable->uBucketIndex];

                                
   uIndex = SymTable_hash(pcKey, uBucketCount);

   
   pbCurrent = oSymTable->ppbBuckets[uIndex];
   

   while (pbCurrent != NULL) {

      if (strcmp(pbCurrent->pcKey, pcKey) == EQUAL)
         return pbCurrent;

      pbCurrent = pbCurrent->pbNext;
   }

   return NULL;
}


void *SymTable_replace(SymTable_T oSymTable, const char *pcKey,
                       const void *pvValue) {

   struct Binding *pbResult;
   void *pvPrevious;

   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   pbResult = SymTable_find(oSymTable, pcKey);

   if (pbResult == NULL) return NULL;

   pvPrevious = pbResult->pvValue;
   pbResult->pvValue = (void*)pvValue;

   return pvPrevious;
   
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {

   
   struct Binding *pbResult;
   enum {NOT_FOUND, FOUND};

   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   pbResult = SymTable_find(oSymTable, pcKey);

   if (pbResult == NULL) return NOT_FOUND;

   return FOUND;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {

   struct Binding *pbResult;

   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   

   pbResult = SymTable_find(oSymTable, pcKey);

   if (pbResult == NULL) return NULL;

   return pbResult->pvValue;
   
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {


   size_t uIndex;
   size_t uBucketCount;
   void *pvValue;
   enum {EQUAL};
   
   struct Binding *pbPrev;
   struct Binding *pbCurrent;


   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   
   uBucketCount = auBucketCount[oSymTable->uBucketIndex];

   
   uIndex = SymTable_hash(pcKey, uBucketCount);


   
   pbCurrent = oSymTable->ppbBuckets[uIndex];
   pbPrev = NULL;

   
   while (pbCurrent != NULL) {

      if(strcmp(pbCurrent->pcKey, pcKey) == EQUAL) {

         /* if Binding is the first on the separate chain */
         if (pbPrev == NULL)
            oSymTable->ppbBuckets[uIndex] = pbCurrent->pbNext;

         else
            pbPrev->pbNext = pbCurrent->pbNext;

         
         pvValue = pbCurrent->pvValue;

         free((void*)pbCurrent->pcKey);
         
         free(pbCurrent);

         oSymTable->uLength--;

         return pvValue;
      }

      pbPrev = pbCurrent;
      pbCurrent = pbCurrent->pbNext;
   }

   return NULL;
}


void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue,
                                  void *pvExtra), const void *pvExtra) {

   size_t i;
   size_t uBucketCount;
   struct Binding *pbCurrent;

   assert(oSymTable != NULL);
   assert(pfApply != NULL);

   
   uBucketCount = auBucketCount[oSymTable->uBucketIndex];

   
   for (i = 0; i < uBucketCount; i++) {

      pbCurrent = oSymTable->ppbBuckets[i];

      
      while (pbCurrent != NULL) {

         (*pfApply)((void*)pbCurrent->pcKey,
                    (void*)pbCurrent->pvValue,
                    (void*)pvExtra);
            
         pbCurrent = pbCurrent->pbNext;
              
      }
   }
}
