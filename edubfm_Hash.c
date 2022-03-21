/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: edubfm_Hash.c
 *
 * Description:
 *  Some functions are provided to support buffer manager.
 *  Each BfMHashKey is mapping to one table entry in a hash table(hTable),
 *  and each entry has an index which indicates a buffer in a buffer pool.
 *  An ordinary hashing method is used and linear probing strategy is
 *  used if collision has occurred.
 *
 * Exports:
 *  Four edubfm_LookUp(BfMHashKey *, Four)
 *  Four edubfm_Insert(BfMHaskKey *, Two, Four)
 *  Four edubfm_Delete(BfMHashKey *, Four)
 *  Four edubfm_DeleteAll(void)
 */


#include <stdlib.h> /* for malloc & free */
#include "EduBfM_common.h"
#include "EduBfM_Internal.h"



/*@
 * macro definitions
 */  

/* Macro: BFM_HASH(k,type)
 * Description: return the hash value of the key given as a parameter
 * Parameters:
 *  BfMHashKey *k   : pointer to the key
 *  Four type       : buffer type
 * Returns: (Two) hash value
 */
#define BFM_HASH(k,type)	(((k)->volNo + (k)->pageNo) % HASHTABLESIZE(type))


/*@================================
 * edubfm_Insert()
 *================================*/
/*
 * Function: Four edubfm_Insert(BfMHashKey *, Two, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Insert a new entry into the hash table.
 *  If collision occurs, then use the linear probing method.
 *
 * Returns:
 *  error code
 *    eBADBUFINDEX_BFM - bad index value for buffer table
 */
Four edubfm_Insert(
    BfMHashKey 		*key,			/* IN a hash key in Buffer Manager */
    Two 		index,			/* IN an index used in the buffer pool */
    Four 		type)			/* IN buffer type */
{
    Four 		i;			
    Two  		hashValue;


    CHECKKEY(key);    /*@ check validity of key */
    if(IS_BAD_BUFFERTYPE(type)) ERR(eBADBUFFERTYPE_BFM);	

    // check if out-of-index
    if( (index < 0) || (index >= BI_NBUFS(type)) )
        ERR( eBADBUFINDEX_BFM );

    /*
    Determine the position in hashTable to insert the array index of the
    buffer element by using the hash value of the 
    page/train residing in the buffer element.
    */
    hashValue = BFM_HASH(key, type);
    i = BI_HASHTABLEENTRY(type, hashValue);
    // if not collision insert the array index into the position determined.
    if( i == NIL ) {
        BI_HASHTABLEENTRY(type, hashValue) = index;

    }
    // If there is a collision, use the chaining method to handle the collision.
    else {
        BI_NEXTHASHENTRY(type, index) = i;
        BI_HASHTABLEENTRY(type, hashValue) = index;
    }

    return( eNOERROR );

}  /* edubfm_Insert */



/*@================================
 * edubfm_Delete()
 *================================*/
/*
 * Function: Four edubfm_Delete(BfMHashKey *, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Look up the entry which corresponds to `key' and
 *  Delete the entry from the hash table.
 *
 * Returns:
 *  error code
 *    eNOTFOUND_BFM - The key isn't in the hash table.
 */
Four edubfm_Delete(
    BfMHashKey          *key,                   /* IN a hash key in buffer manager */
    Four                type )                  /* IN buffer type */
{
    Two                 i, prev;                
    Two                 hashValue;


    CHECKKEY(key);    /*@ check validity of key */

    /*
    Search for the array index of the buffer element containing the
    page/train to be deleted from hashTable by using the hash value
    of the page/train residing in the buffer element.
    */
    hashValue = BFM_HASH (key, type);

    prev = NIL;
    
    i = BI_HASHTABLEENTRY(type, hashValue);

    if ( i == NIL ) {
        ERR( eNOTFOUND_BFM );
    }
     while (i != NIL) {
        // Delete the entry (array index) found from hashTable
        if (EQUALKEY(&BI_KEY(type, i), key)) {
            // No next & No prev
            if( BI_NEXTHASHENTRY(type, i) == NIL && prev == NIL ) {
                BI_HASHTABLEENTRY(type, hashValue) = NIL;
            }
            // No next & Yes prev
            else if ( BI_NEXTHASHENTRY(type, i) == NIL ) {
                BI_NEXTHASHENTRY(type, prev) = NIL;
            }
            //  Yes next & No prev
            else if (BI_NEXTHASHENTRY(type, i) != NIL && prev == NIL ) {
                BI_HASHTABLEENTRY(type, hashValue) = BI_NEXTHASHENTRY(type, i);
            }
            // Yes next & Yes prev
            else {
                BI_NEXTHASHENTRY(type, prev) = BI_NEXTHASHENTRY(type, i);
            }
            return( eNOERROR );
        }

        prev = i;
        i = BI_NEXTHASHENTRY(type, i);
    }
    return( eNOERROR );
}  /* edubfm_Delete */



/*@================================
 * edubfm_LookUp()
 *================================*/
/*
 * Function: Four edubfm_LookUp(BfMHashKey *, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Look up the given key in the hash table and return its
 *  corressponding index to the buffer table.
 *
 * Retruns:
 *  index on buffer table entry holding the train specified by 'key'
 *  (NOTFOUND_IN_HTABLE - The key don't exist in the hash table.)
 */
Four edubfm_LookUp(
    BfMHashKey          *key,                   /* IN a hash key in Buffer Manager */
    Four                type)                   /* IN buffer type */
{
    Two                 i, j;                   /* indices */
    Two                 hashValue;


    CHECKKEY(key);    /*@ check validity of key */

    /*
    Search for the array index of the buffer element having the
    page/train whose hash key (BfMHashKey) is the same with
    that given as a parameter from hashTable, and return it.
    */
    hashValue = BFM_HASH(key, type);

    i = BI_HASHTABLEENTRY(type, hashValue);

    /*
    Using the hash key, search for the array index of the buffer
    element containing the page/train having the hash key from
    hashTable.
    */
    while ( i != NIL ) {
       
        if (EQUALKEY(&BI_KEY(type, i), key))
        //Return the array index found.
            return i;
        
        i = BI_NEXTHASHENTRY(type, i);
    }
    // if i = NIL initially
    return( NOTFOUND_IN_HTABLE );
}  /* edubfm_LookUp */



/*@================================
 * edubfm_DeleteAll()
 *================================*/
/*
 * Function: Four edubfm_DeleteAll(void)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Delete all hash entries.
 *
 * Returns:
 *  error code
 */
Four edubfm_DeleteAll(void)
{
    Two 		i;
    Four 		type, tableSize;
    
    /*
    Delete every entry (array index of the buffer element) from
    each hashTable. 
    */

    for ( type = 0; type < NUM_BUF_TYPES; type++) {
        tableSize = HASHTABLESIZE(type);

        for (i = 0; i < tableSize; i++) {
            BI_HASHTABLEENTRY(type, i) = NIL;
        }
    }

    return( eNOERROR );

} /* edubfm_DeleteAll() */ 
