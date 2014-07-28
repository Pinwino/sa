#include <stdio.h>
#include <stdlib.h>
#include <pp-printf.h>
#define GGC_MIN_HEAPSIZE_DEFAULT 32 /* 4096 by default */


/* form ram.ld */
extern unsigned int _HEAP_START;
extern unsigned int _HEAP_END;
extern unsigned int aling;
extern unsigned int _endram;
extern unsigned int _fstack;
caddr_t heap = NULL;


/*
 * I should include some incomprehensible software devepoler bullshit here
 * but I am a hardware/gatewary gay, I'll just say: needed by malloc
 *
 */
caddr_t _sbrk ( int increment ) {

    caddr_t prevHeap;
    caddr_t nextHeap;

    if (heap == NULL) {
        /* first allocation is at heap start */
        heap = (caddr_t)&_HEAP_START; 
        /*_HEAP_START value, &_HEAP_START address, be carefull with that!!! */
    }

    prevHeap = heap;

   /*
    * Although it is recommended to return data aligned on a 4 byte boundary,
    *
    * nextHeap = (caddr_t)(((unsigned int)(heap + increment) + aling) & ~aling)
    * 
    * in our case, we don't need to do it
    */

	nextHeap = (caddr_t)((unsigned int)(heap + increment));
	register caddr_t stackPtr asm ("sp");

    /*
     * What this thig intends to do is to check if it is enough space
     * or a collision with stack coming the other way takes place.
     * Our stack is above start of heap.
     */
    if ((((caddr_t)&_HEAP_START < stackPtr) && (nextHeap > stackPtr)) ||
         (nextHeap >= (caddr_t)&_HEAP_END)) {
		return NULL; /* the heap bitch ran out of space - no more memory */
    } else {
        heap = nextHeap;
        if (0)
			pp_printf("size %u\n", (unsigned int)(heap)-(unsigned int)(&_HEAP_START));
        return (caddr_t) prevHeap;
    }
}
