/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* author: Julio Lins (jcclb)                                         */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/


static const size_t auBucketCount[] = {509, 1021, 2039, 4093, 8191,
                                       16381, 32749, 65521};


struct Binding {
   const char *pcKey;
   void *pvValue;
   struct Binding *pbNext;
};


struct SymTable {
   size_t uBucketIndex;
   struct Binding **ppbBuckets;
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
   inclusive. */

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


static size_t SymTable_nextBucket(size_t uCurrentIndex) {

   const size_t MAX_INDEX = sizeof(auBucketCount)/sizeof(size_t) - 1;

   if (uCurrentIndex == MAX_INDEX)
      return uCurrentIndex;

   else
      return ++uCurrentIndex;
}


static SymTable_T SymTable_grow(SymTable_T oSymTable) {

   size_t uNextIndex;
   size_t uCurrentIndex = oSymTable->uBucketIndex;
   SymTable_T oTempSymTable;
   struct Binding* pbCurrent;
   struct Binding** ppbTemp;
   size_t i;


   uNextIndex = SymTable_nextBucket(uCurrentIndex);


   oTempSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
   

   if (oTempSymTable == NULL)
      return oSymTable;

   
   oTempSymTable->ppbBuckets =
      (struct Binding**)calloc(sizeof(struct Binding*),
                               auBucketCount[uNextIndex]);

   if (oTempSymTable->ppbBuckets == NULL) {

      free(oTempSymTable);
      return oSymTable;
   }

  
   oTempSymTable->uLength = 0;
   oTempSymTable->uBucketIndex = uNextIndex;

   
   for (i = 0; i < auBucketCount[uCurrentIndex]; i++) {

      
      pbCurrent = oSymTable->ppbBuckets[i];

      
      while (pbCurrent != NULL) {

         SymTable_put(oTempSymTable, pbCurrent->pcKey,
                      pbCurrent->pvValue);

         pbCurrent = pbCurrent->pbNext;

      }
   }

   /* REVIEW */

   ppbTemp = oSymTable->ppbBuckets;
   oSymTable->ppbBuckets = oTempSymTable->ppbBuckets;
   oTempSymTable->ppbBuckets = ppbTemp;


   oSymTable->uBucketIndex = uNextIndex;
   oTempSymTable->uBucketIndex = uCurrentIndex;
   
   SymTable_free(oTempSymTable);

   return oSymTable;
}


int SymTable_put(SymTable_T oSymTable, const char *pcKey,
                 const void *pvValue) {


   size_t uIndex;
   char *pcCopy;
   struct Binding *pbCurrent;
   struct Binding *pbNewBinding;
   enum {FALSE, TRUE};
   enum {EQUAL};

   
   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   
   if (oSymTable->uLength == auBucketCount[oSymTable->uBucketIndex])
      oSymTable = SymTable_grow(oSymTable);

   
   uIndex = SymTable_hash(pcKey,
                          auBucketCount[oSymTable->uBucketIndex]);


   
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


static struct Binding *SymTable_find(SymTable_T oSymTable,
                                     const char *pcKey) {

   size_t uIndex;
   struct Binding *pbCurrent;
   enum {EQUAL};

   
   uIndex = SymTable_hash(pcKey,
                          auBucketCount[oSymTable->uBucketIndex]);

   
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


   struct Binding *pbPrev;
   struct Binding *pbCurrent;
   void *pvValue;
   enum {EQUAL};
   size_t uIndex;


   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   
   uIndex = SymTable_hash(pcKey,
                          auBucketCount[oSymTable->uBucketIndex]);


   
   pbCurrent = oSymTable->ppbBuckets[uIndex];
   pbPrev = NULL;

   
   while (pbCurrent != NULL) {

      if(strcmp(pbCurrent->pcKey, pcKey) == EQUAL) {

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
   struct Binding *pbCurrent;

   assert(oSymTable != NULL);
   assert(pfApply != NULL);


   for (i = 0; i < auBucketCount[oSymTable->uBucketIndex]; i++) {


      pbCurrent = oSymTable->ppbBuckets[i];

      
      while (pbCurrent != NULL) {

         (*pfApply)((void*)pbCurrent->pcKey,
                    (void*)pbCurrent->pvValue,
                    (void*)pvExtra);
            
         pbCurrent = pbCurrent->pbNext;
              
      }
   }
}