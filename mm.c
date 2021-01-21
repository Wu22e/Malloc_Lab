/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


#define NEXT_FITx

 /*********************************************************
  * NOTE TO STUDENTS: Before you do anything else, please
  * provide your team information in the following struct.
  ********************************************************/

team_t team = {
    /* Team name */
    "team 8",
    /* First member's full name */
    "Hyungwoo Kim",
    /* First member's email address */
    "gulb1602@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};


/* Basic constants and macros */

#define WSIZE     4                                         /* Word and header/footer size (bytes), size in bytes of a single word, �� ���� ũ�� */
#define DSIZE     8                                         /* Double word size (bytes), size in bytes of a double word, ���� ���� ũ�� */         

//#define INITSIZE          16                                /* Initial size of free list before first free block added, �ʱ� �� ũ�� */
//#define MINBLOCKSIZE      16                                /* Minmum size for a free block, includes 4 bytes for header/footer                                                            free blocks, �ּ� ��� ũ�� */

#define MINIMUM           24
#define CHUNKSIZE         16



#define MAX(x, y) ((x) > (y)? (x) : (y))                    /* x�� y�� ū ���� ���Ѵ�. */

                                                               /* Pack a size and allocated bit into a word */
#define PACK(size, alloc)   ((size)|(alloc))                /* ũ��� allocated ��Ʈ�� ��ģ��. ��, header�� footer�� ���ȴ�. */

/* Read and write a word at address p */
#define GET(p)  (*(unsigned int *)(p))                      /* �ּ� p�� ����Ű�� ���� �ִ� ���� �о�´�. */    
#define PUT(p, val)  (*(unsigned int *)(p) = (val))         /* �ּ� p�� �� val�� �����Ѵ�. */

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                                                /* p�����Ͱ� ����Ű�� ���� size�� ������(�������� ������) */
#define GET_ALLOC(p)  (GET(p) & 0x1)                                                /* p�����Ͱ� ����Ű�� ���� allocation ������ ������(1 �Ǵ� 0) */

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)    ((char *)(bp) - WSIZE)                                          /* ��� �����Ϳ��� �������� word size(4 byte) ��ŭ �̵��� �����͸� ��ȯ(����� ����� ����Ű�� ������) */
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp))-DSIZE)                       /* ��������Ϳ��� ������ ����� ��������Ϳ� �����ص� DSIZE��ŭ ��ĭ ����(����� ǲ�͸� ����Ű�� ������) */

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))            /* ���� ����� ��������͸� ��ȯ */
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))            /* ���� ����� ��������͸� ��ȯ */

/* Given block ptr bp, compute address of next and previous free blocks */
#define PREV_FREE(bp) (*(void **)(bp))                                      /* ��� ������ bp�� ���� ���� ����� ����� */
#define NEXT_FREE(bp) (*(void **)(bp + WSIZE))                                              /* ��� ������ bp�� ���� ���� ����� ����� */


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8                                             // memory alignment factor

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Global variables, Private variables representing the heap and free list within the heap */
static char* heap_listp; /* Pointer to first block */

static char* free_listp; /* explicit method, ù ��° ���� ����� �����ͷ� ���� */

#ifdef NEXT_FIT
static char* next_ptr; /* Next fit rover */
#endif

// PROTOTYPES
static void* extend_heap(size_t words);
static void* find_fit(size_t size);
static void* coalesce(void* bp);
static void place(void* bp, size_t asize);
static void removeFreeBlock(void* bp);
// static int mm_check();


/**
 *	@fn		mm_init
 *
 *	@brief                  Initialize the heap with freelist prologue/epilogoue and space for the
 *                          initial free block. (32 bytes total)
 *                          freelist�� �̿��Ͽ� heap�� �ʱ�ȭ�Ѵ�. �̶� 32bytes��ŭ�� heap�� ����
 *                          prologue�� epilogoue �׸��� header, next, prev, footer pointer�� �������ش�.
 *
 *  @param  void
 *
 *	@return	int             initialize: return -1 on error, 0 on success.
 */

 /*
  * mm_init - Initializes the heap like that shown below.
  *
  * FREE BLOCK
  * |------------|------------|------------|-------------|------------|
  * |   PADDING  |   HEADER   |   PREV     |   NEXT      |   FOOTER   |
  * |------------|------------|------------|-------------|------------|
  * ^                         ^
  * heap_listp                bp = free_listp
  */

int mm_init(void)
{

    /* Create the initial empty heap */
    /* �޸𸮿� �ʱ� empty heap�� �䱸�Ѵ�. */
    //if ((heap_listp = mem_sbrk(2*MINIMUM)) == (void*)-1)
    if ((heap_listp = mem_sbrk(3 * DSIZE)) == (void*)-1)

        return -1;

    /* ó�� ��� �κ��� �ʱ�ȭ �Ѵ�. */
    PUT(heap_listp, 0);                                // Padding
    PUT(heap_listp + (1 * WSIZE), PACK(MINIMUM, 1));                  // Free block header
    PUT(heap_listp + (2 * WSIZE), PACK(0, 0));                             // Space for prev pointer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 0));                             // Space for next pointer
    PUT(heap_listp + (4 * WSIZE), PACK(MINIMUM, 1));                  // Free block footer
    /* epilogue ����� �ʱ�ȭ �Ѵ�. */
    PUT(heap_listp + (5 * WSIZE), PACK(0, 1));                             // Epilogue header

    // ���� ����Ʈ �����͸� �ʱ�ȭ �Ѵ�.
    free_listp = heap_listp + DSIZE;

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

#ifdef NEXT_FIT
    next_ptr = heap_listp;
#endif
    return 0;
}



/**
 *	@fn		mm_malloc
 *
 *	@brief                  Allocate a block by incrementing the brk pointer.
 *                         Always allocate a block whose size is a multiple of the alignment.
 *
 *                         A block is allocated according to this strategy:
 *                         (1) If a free block of the given size is found, then allocate that free block and return
 *                         a pointer to the payload of that block.
 *                         (2) Otherwise a free block could not be found, so an extension of the heap is necessary.
 *                         Simply extend the heap and place the allocated block in the new free block.
 *
 * @param  size           �Ű����� size�� �Ѱܹ޾� �� ũ�⸸ŭ ����� �Ҵ���
 *
 *	@return void*          initialize: return -1 on error, 0 on success.
 */

void* mm_malloc(size_t size)
{
    /* size�� �ùٸ��� ���� �� ����ó�� */
    /* size�� ���� ���� ���� �����Ѵ�. */
    if (size <= 0)
        return NULL;

    size_t asize;                                           /* Adjusted block size, ������ ��� ũ�⸦ ���� ���� */
    size_t extendsize;                                      /* Amount to extend heap if no fit, �´� fit�� ���� ��� ���� �ø��� �� */
    char* bp;


    /* block�� ũ�� ���� */
    /* �������� ������ ���� �ʿ��� �κ��� �����ؼ� ũ�⸦ �����Ѵ�. */
    /* ��� �ּ� ��� ũ�⸦ ������, ��ũ�η� ������ MAX�� �̿��Ͽ� ũ�⸦ �����Ѵ�. */
    asize = MAX(ALIGN(size) + DSIZE, MINIMUM);

    /* Search the free list for a fit */
    /* ������ �������� ���븮��Ʈ���� �˻�, ������ ũ�⿡ �˸��� ����� list���� �˻��Ͽ� �ش� ��ġ�� �Ҵ��Ѵ�. */
    if ((bp = find_fit(asize))) {
        place(bp, asize);
        /* �´� ��� ã���� �Ҵ��� ��û�� ��� ��ġ�ϰ�, �ɼ����� �ʰ��κ��� ���� */
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    /* ���� ���ο� ���� ������� Ȯ��, free list���� ������ ����� ã�� �������� ���� �÷��� �Ҵ��ϰ�, �װ��� ����� �Ҵ��Ѵ�. */
    extendsize = MAX(asize, MINIMUM); // ���� asize���� üũ���ִϱ� �ʿ����������?

    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;

    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* bp)
{
    if (!bp) return;
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * remove_freeblock - Removes the given free block pointed to by bp from the free list.
 *
 * The explicit free list is simply a doubly linked list. This function performs a removal
 * of the block from the doubly linked free list.
 */

 // ���� ����Ʈ���� ����� �����Ѵ�
static void removeFreeBlock(void* bp)
{
    if (bp) {
        /* ���� ���� ����� �ִٸ�, �� ����� next�� ������ ���� ���� ����� ����Ű���� �Ѵ�. */
        if (PREV_FREE(bp)) {
            NEXT_FREE(PREV_FREE(bp)) = NEXT_FREE(bp);
        }
        /* �׷��� ������, ���� ����Ʈ�� ó��(free_listp)�� ���� ���� ����� ����Ű���� �Ѵ�. */
        else
            free_listp = NEXT_FREE(bp);

        /* ���� ����� prev�� bp�� prev�� ����Ű�� ���� ����Ű���� �Ѵ�. */
        if (NEXT_FREE(bp) != NULL)
            PREV_FREE(NEXT_FREE(bp)) = PREV_FREE(bp);
    }

}


static void* find_fit(size_t size)
{


#ifdef NEXT_FIT 
    /* Next fit search */
    char* bp = next_ptr;

    /* Search from the next_ptr to the end of list */
    for (; GET_SIZE(HDRP(next_ptr)) > 0; next_ptr = NEXT_BLKP(next_ptr))
        if (!GET_ALLOC(HDRP(next_ptr)) && (asize <= GET_SIZE(HDRP(next_ptr))))
            return next_ptr;

    /* search from start of list to old ptr(= heap_listp) */
    for (next_ptr = heap_listp; next_ptr < bp; next_ptr = NEXT_BLKP(next_ptr))
        if (!GET_ALLOC(HDRP(next_ptr)) && (asize <= GET_SIZE(HDRP(next_ptr))))
            return next_ptr;

    return NULL;  /* no fit found */


   ///* Next-fit search (�������� �ڵ�)*/
   // void* bp;
   // for (bp = next_ptr; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
   //     if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
   //         //lowers the performance
   //         //next_start = bp;
   //         return bp;
   //     }
   // }
   // for (bp = heap_listp; bp != next_ptr; bp = NEXT_BLKP(bp)) {
   //     if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
   //         //lowers the performance
   //         //next_start = bp;
   //         return bp;
   //     }
   // }
   // return NULL; /* No fit */
#else 
    /* $begin mmfirstfit */
        /* First fit search */
    void* bp;

    for (bp = free_listp; GET_ALLOC(HDRP(bp)) == 0; bp = NEXT_FREE(bp)) {
        if (size <= (size_t)GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    return NULL; /* No fit */
/* $end mmfirstfit */
#endif
}




static void place(void* bp, size_t asize)
{
    size_t fsize = GET_SIZE(HDRP(bp)); // free size

    if ((fsize - asize) >= MINIMUM) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        removeFreeBlock(bp);
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(fsize - asize, 0));
        PUT(FTRP(bp), PACK(fsize - asize, 0));
        coalesce(bp);
    }
    else {
        PUT(HDRP(bp), PACK(fsize, 1));
        PUT(FTRP(bp), PACK(fsize, 1));
        removeFreeBlock(bp);
    }
}


static void* coalesce(void* bp)
{

    /* ���� ��ϰ� ���� ����� �Ҵ�Ǿ� �ִ��� �˾ƺ��� ������ */
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* ���� ���, ���� ��� �Ҵ��� ��� */
    // �ƹ� �͵� ���� ����. ���� �ش� ����� ���븮��Ʈ �� �տ� ����

    /* ���� ��� �Ҵ�, ���� ��� ������ ��� */
    if (prev_alloc && !next_alloc) {                              /* Case 1 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        removeFreeBlock(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) {                        /* Case 2 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        removeFreeBlock(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && !next_alloc) {                        /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        removeFreeBlock(PREV_BLKP(bp));
        removeFreeBlock(NEXT_BLKP(bp));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    NEXT_FREE(bp) = free_listp;
    PREV_FREE(free_listp) = bp;
    PREV_FREE(bp) = NULL;
    free_listp = bp;

    return bp;
}

/**
 *	@fn		extend_heap
 *
 *	@brief                  ���� ����� ���� �ø���, �װ��� ��� �����͸� ��ȯ�Ѵ�.
 *
 *  @param  words           words ũ���� �����ŭ ���� �÷��ش�.
 *
 *	@return	static void*
 */

static void* extend_heap(size_t words)
{
    char* bp;                   /* ���� Ȯ���� �Ŀ� ���ο� ��� ������ */
    size_t asize;               /* �� �޸𸮿� size�� �䱸 */

    /* Allocate an even number of words to mainitain alignment */
    /* ������ �����ϱ� ���� words�� ���� ¦���� �����. */
    asize = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if (asize < MINIMUM)
        asize = MINIMUM;

    if ((long)(bp = mem_sbrk(asize)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    /* ���� ����� header�� footer, epilogue�� �ʱ�ȭ �Ѵ�. */
    PUT(HDRP(bp), PACK(asize, 0));                                       /* Free block header */
    PUT(FTRP(bp), PACK(asize, 0));                                       /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));                               /* New epilogue header */

    /* Coalesce if the previous block was free */
    /* ���� ����� ���� ����̾��ٸ� �պ��� �ϰ�, ���븮��Ʈ�� ����� �߰��Ѵ�. */
    return coalesce(bp);
}








/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void* mm_realloc(void* ptr, size_t size)
{

    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size <= 0) {
        mm_free(ptr);
        return NULL;
    }



    size_t asize = MAX(ALIGN(size) + DSIZE, MINIMUM);
    size_t current_size = GET_SIZE(HDRP(ptr));

    void* bp;
    char* next = HDRP(NEXT_BLKP(ptr));
    size_t newsize = current_size + GET_SIZE(next);


    if (asize == current_size)
        return ptr;

    if (asize <= current_size) {
        if (asize > MINIMUM && (current_size - asize) > MINIMUM) {
            PUT(HDRP(ptr), PACK(asize, 1));
            PUT(FTRP(ptr), PACK(asize, 1));
            bp = NEXT_BLKP(ptr);
            PUT(HDRP(bp), PACK(current_size - asize, 1));
            PUT(FTRP(bp), PACK(current_size - asize, 1));
            mm_free(bp);
            return ptr;
        }

        bp = mm_malloc(asize);
        memcpy(bp, ptr, asize);
        mm_free(ptr);

        return bp;
    }

    else {
        if (!GET_ALLOC(next) && newsize >= asize) {
            removeFreeBlock(NEXT_BLKP(ptr));
            PUT(HDRP(ptr), PACK(asize, 1));
            PUT(FTRP(ptr), PACK(asize, 1));
            bp = NEXT_BLKP(ptr);
            PUT(HDRP(bp), PACK(newsize - asize, 1));
            PUT(FTRP(bp), PACK(newsize - asize, 1));
            mm_free(bp);
            return ptr;
        }

        bp = mm_malloc(asize);
        memcpy(bp, ptr, current_size);
        mm_free(ptr);
        return bp;
    }

}

////consistency checker

//static int mm_check() {

//  // Is every block in the free list marked as free?
//  void *next;
//  for (next = free_listp; GET_ALLOC(HDRP(next)) == 0; next = NEXT_FREE(next)) {
//    if (GET_ALLOC(HDRP(next))) {
//      printf("Consistency error: block %p in free list but marked allocated!", next);
//      return 1;
//    }
//  }

//  // Are there any contiguous free blocks that escaped coalescing?
//  for (next = free_listp; GET_ALLOC(HDRP(next)) == 0; next = NEXT_FREE(next)) {

//    char *prev = PREV_FREE(HDRP(next));
//      if(prev != NULL && HDRP(next) - FTRP(prev) == DSIZE) {
//        printf("Consistency error: block %p missed coalescing!", next);
//        return 1;
//      }
//  }

//  // Do the pointers in the free list point to valid free blocks?
//  for (next = free_listp; GET_ALLOC(HDRP(next)) == 0; next = NEXT_FREE(next)) {

//    if(next < mem_heap_lo() || next > mem_heap_hi()) {
//      printf("Consistency error: free block %p invalid", next);
//      return 1;
//    }
//  }

//  // Do the pointers in a heap block point to a valid heap address?
//  for (next = heap_listp; NEXT_BLKP(next) != NULL; next = NEXT_BLKP(next)) {

//    if(next < mem_heap_lo() || next > mem_heap_hi()) {
//      printf("Consistency error: block %p outside designated heap space", next);
//      return 1;
//    }
//  }

//  return 0;
//}