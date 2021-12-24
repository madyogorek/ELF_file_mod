// el_malloc.c: implementation of explicit list malloc functions.
//Madelyn Ogorek ogore014 5454524 CSCI 2021

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "el_malloc.h"

////////////////////////////////////////////////////////////////////////////////
// Global control functions

// Global control variable for the allocator. Must be initialized in
// el_init().
el_ctl_t el_ctl = {};

// Create an initial block of memory for the heap using
// malloc(). Initialize the el_ctl data structure to point at this
// block. Initialize the lists in el_ctl to contain a single large
// block of available memory and no used blocks of memory.
int el_init(int max_bytes){
  void *heap = malloc(max_bytes);
  if(heap == NULL){
    fprintf(stderr,"el_init: malloc() failed in setup\n");
    exit(1);
  }

  el_ctl.heap_bytes = max_bytes; // make the heap as big as possible to begin with
  el_ctl.heap_start = heap;      // set addresses of start and end of heap
  el_ctl.heap_end   = PTR_PLUS_BYTES(heap,max_bytes);

  if(el_ctl.heap_bytes < EL_BLOCK_OVERHEAD){
    fprintf(stderr,"el_init: heap size %ld to small for a block overhead %ld\n",
            el_ctl.heap_bytes,EL_BLOCK_OVERHEAD);
    return 1;
  }

  el_init_blocklist(&el_ctl.avail_actual);
  el_init_blocklist(&el_ctl.used_actual);
  el_ctl.avail = &el_ctl.avail_actual;
  el_ctl.used  = &el_ctl.used_actual;

  // establish the first available block by filling in size in
  // block/foot and null links in head
  size_t size = el_ctl.heap_bytes - EL_BLOCK_OVERHEAD;
  el_blockhead_t *ablock = el_ctl.heap_start;
  ablock->size = size;
  ablock->state = EL_AVAILABLE;
  el_blockfoot_t *afoot = el_get_footer(ablock);
  afoot->size = size;
  el_add_block_front(el_ctl.avail, ablock);
  return 0;
}

// Clean up the heap area associated with the system which simply
// calls free() on the malloc'd block used as the heap.
void el_cleanup(){
  free(el_ctl.heap_start);
  el_ctl.heap_start = NULL;
  el_ctl.heap_end   = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Pointer arithmetic functions to access adjacent headers/footers

// Compute the address of the foot for the given head which is at a
// higher address than the head.
el_blockfoot_t *el_get_footer(el_blockhead_t *head){
  size_t size = head->size;
  el_blockfoot_t *foot = PTR_PLUS_BYTES(head, sizeof(el_blockhead_t) + size);
  return foot;
}

// REQUIRED
// Compute the address of the head for the given foot which is at a
// lower address than the foot.
el_blockhead_t *el_get_header(el_blockfoot_t *foot){
  size_t size = foot->size;
  //literally doing the reverse arithmetic that get_footer did to get back to the header
  void *head = PTR_MINUS_BYTES(foot, sizeof(el_blockhead_t) + size);
  //gets us back up to the head
  return head;
}

// Return a pointer to the block that is one block higher in memory
// from the given block.  This should be the size of the block plus
// the EL_BLOCK_OVERHEAD which is the space occupied by the header and
// footer. Returns NULL if the block above would be off the heap.
// DOES NOT follow next pointer, looks in adjacent memory.
el_blockhead_t *el_block_above(el_blockhead_t *block){
  //getting to the header of the block above
  el_blockhead_t *higher =
    PTR_PLUS_BYTES(block, block->size + EL_BLOCK_OVERHEAD);
    //preventing a segfault by making sure we don't go off the heap
  if((void *) higher >= (void*) el_ctl.heap_end){
    //we know we can't go any further in memory
    return NULL;
  }
  else{
    //valid node
    return higher;
  }
}

// REQUIRED
// Return a pointer to the block that is one block lower in memory
// from the given block.  Uses the size of the preceding block found
// in its foot. DOES NOT follow block->next pointer, looks in adjacent
// memory. Returns NULL if the block below would be outside the heap.
//
// WARNING: This function must perform slightly different arithmetic
// than el_block_above(). Take care when implementing it.
el_blockhead_t *el_block_below(el_blockhead_t *block){
  //moving over a little bit to get the footer of the adjacent
  el_blockfoot_t *footy = PTR_MINUS_BYTES(block, sizeof(el_blockfoot_t));
  //preventing us from calling get_header with an invalid address
  if((void *) footy < (void *) el_ctl.heap_start){
    return NULL;}
  el_blockhead_t *lower = el_get_header(footy);
  //preventing us from returning an invalid address
  if((void *) lower < (void *) el_ctl.heap_start){
    return NULL;}
  else
  {
    return lower;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Block list operations

// Print an entire blocklist. The format appears as follows.
//
// blocklist{length:      5  bytes:    566}
//   [  0] head @    618 {state: u  size:    200}  foot @    850 {size:    200}
//   [  1] head @    256 {state: u  size:     32}  foot @    320 {size:     32}
//   [  2] head @    514 {state: u  size:     64}  foot @    610 {size:     64}
//   [  3] head @    452 {state: u  size:     22}  foot @    506 {size:     22}
//   [  4] head @    168 {state: u  size:     48}  foot @    248 {size:     48}
//   index        offset        a/u                       offset
//
// Note that the '@ offset' column is given from the starting heap
// address (el_ctl->heap_start) so it should be run-independent.
void el_print_blocklist(el_blocklist_t *list){
  printf("blocklist{length: %6lu  bytes: %6lu}\n", list->length,list->bytes);
  el_blockhead_t *block = list->beg;
  for(int i=0; i<list->length; i++){
    printf("  ");

    block = block->next;

    printf("[%3d] head @ %6lu ", i,PTR_MINUS_PTR(block,el_ctl.heap_start));

    printf("{state: %c  size: %6lu}", block->state,block->size);

    el_blockfoot_t *foot = el_get_footer(block);

    printf("  foot @ %6lu ", PTR_MINUS_PTR(foot,el_ctl.heap_start));
    printf("{size: %6lu}", foot->size);
    printf("\n");
  }

}

// Print out basic heap statistics. This shows total heap info along
// with the Available and Used Lists. The output format resembles the following.
//
// HEAP STATS
// Heap bytes: 1024
// AVAILABLE LIST: blocklist{length:      3  bytes:    458}
//   [  0] head @    858 {state: a  size:    126}  foot @   1016 {size:    126}
//   [  1] head @    328 {state: a  size:     84}  foot @    444 {size:     84}
//   [  2] head @      0 {state: a  size:    128}  foot @    160 {size:    128}
// USED LIST: blocklist{length:      5  bytes:    566}
//   [  0] head @    618 {state: u  size:    200}  foot @    850 {size:    200}
//   [  1] head @    256 {state: u  size:     32}  foot @    320 {size:     32}
//   [  2] head @    514 {state: u  size:     64}  foot @    610 {size:     64}
//   [  3] head @    452 {state: u  size:     22}  foot @    506 {size:     22}
//   [  4] head @    168 {state: u  size:     48}  foot @    248 {size:     48}
void el_print_stats(){
  printf("HEAP STATS\n");
  printf("Heap bytes: %lu\n",el_ctl.heap_bytes);
  printf("AVAILABLE LIST: ");
  el_print_blocklist(el_ctl.avail);
  printf("USED LIST: ");
  el_print_blocklist(el_ctl.used);
}

// Initialize the specified list to be empty. Sets the beg/end
// pointers to the actual space and initializes those data to be the
// ends of the list.  Initializes length and size to 0.
void el_init_blocklist(el_blocklist_t *list){
  list->beg        = &(list->beg_actual);
  list->beg->state = EL_BEGIN_BLOCK;
  list->beg->size  = EL_UNINITIALIZED;
  list->end        = &(list->end_actual);
  list->end->state = EL_END_BLOCK;
  list->end->size  = EL_UNINITIALIZED;
  list->beg->next  = list->end;
  list->beg->prev  = NULL;
  list->end->next  = NULL;
  list->end->prev  = list->beg;
  list->length     = 0;
  list->bytes      = 0;
}

// REQUIRED
// Add to the front of list; links for block are adjusted as are links
// within list.  Length is incremented and the bytes for the list are
// updated to include the new block's size and its overhead.
void el_add_block_front(el_blocklist_t *list, el_blockhead_t *block){
  //incrementing list's length
  list->length++;
  //adding the new bytes
  list->bytes += block->size + EL_BLOCK_OVERHEAD;
  //linking the node's previous field
  block->prev = list->beg;
  //linking the node's next field
  block->next = list->beg->next;
  //linking the beginning dummy node's next pointer
  block->prev->next = block;
  //linking the original first node's previous field to the new node
  block->next->prev = block;
}

// REQUIRED
// Unlink block from the list it is in which should be the list
// parameter.  Updates the length and bytes for that list including
// the EL_BLOCK_OVERHEAD bytes associated with header/footer.
void el_remove_block(el_blocklist_t *list, el_blockhead_t *block){
  //decrementing length
  list->length--;
  //deleting bytes
  list->bytes -= block->size + EL_BLOCK_OVERHEAD;
  //mending the gap
  block->prev->next = block->next;
  //mending the gap
  block->next->prev = block->prev;
}

////////////////////////////////////////////////////////////////////////////////
// Allocation-related functions

// REQUIRED
// Find the first block in the available list with block size of at
// least (size+EL_BLOCK_OVERHEAD). Overhead is accounted so this
// routine may be used to find an available block to split: splitting
// requires adding in a new header/footer. Returns a pointer to the
// found block or NULL if no of sufficient size is available.
el_blockhead_t *el_find_first_avail(size_t size){
  //keeping track of how many times I step through the list
  int count = 0;
  el_blockhead_t *temp = el_ctl.avail->beg->next;
  //geting the first actual node
  while(count <= el_ctl.avail->length)
  {
    //seeing if the current node has enough space for my malloc
    if(temp->size >= (size + EL_BLOCK_OVERHEAD))
    {
      //returnin pointer to that head
      return temp;
    }
    else
    {
      //progressing through the list
      temp = temp->next;
      count++;
    }
  }
  //no space was found
  return NULL;
}

// REQUIRED
// Set the pointed to block to the given size and add a footer to
// it. Creates another block above it by creating a new header and
// assigning it the remaining space. Ensures that the new block has a
// footer with the correct size. Returns a pointer to the newly
// created block while the parameter block has its size altered to
// parameter size. Does not do any linking of blocks.  If the
// parameter block does not have sufficient size for a split (at least
// new_size + EL_BLOCK_OVERHEAD for the new header/footer) makes no
// changes and returns NULL.
el_blockhead_t *el_split_block(el_blockhead_t *block, size_t new_size){
  //block is too small to do anything
  if(block->size < new_size + EL_BLOCK_OVERHEAD)
  {
    return NULL;
  }
  //for calculating the remaining size
  size_t oldSize = block->size;
  //updating the size of the block to be what was malloc'd
  block->size = new_size;
  //the leftovers will be set to the reamining size
  size_t remainingSize = oldSize - new_size;
  //matching the new blocks size in the foot
  el_blockfoot_t *footold = el_get_footer(block);
  footold->size = new_size;
  //matching the leftover block's size with remaining size
  el_blockhead_t *newHeader = el_block_above(block);
  newHeader->size = remainingSize - EL_BLOCK_OVERHEAD;
  el_blockfoot_t *footnew = el_get_footer(newHeader);
  footnew->size = remainingSize - EL_BLOCK_OVERHEAD;
  //we turned 1 block into 2 so you need to increment length
  el_ctl.avail->length++;
  return newHeader;
}

// REQUIRED
// Return pointer to a block of memory with at least the given size
// for use by the user.  The pointer returned is to the usable space,
// not the block header. Makes use of find_first_avail() to find a
// suitable block and el_split_block() to split it.  Returns NULL if
// no space is available.
void *el_malloc(size_t nbytes){
  //finding space
  el_blockhead_t *myHead = el_find_first_avail(nbytes);
  //checking to make sure there is space availible
  if(myHead == NULL)
  {
    return NULL;
  }
  //attempting to split
  el_blockhead_t *newHead = el_split_block(myHead, nbytes);
  //if the split was successful you have to update the pointers
  if(newHead != NULL)
  {
    myHead->next->prev = newHead;
    newHead->next = myHead->next;
    myHead->next = newHead;
    newHead->prev = myHead;
    //removing both blocks
    el_remove_block(el_ctl.avail, myHead);
    el_remove_block(el_ctl.avail, newHead);
    //adding back to the front of the list
    el_add_block_front(el_ctl.avail, newHead);
    //updating the states
    newHead->state = EL_AVAILABLE;
    myHead->state = EL_USED;
    //putting the malloc'd stuff in used
    el_add_block_front(el_ctl.used, myHead);
    //returning the usable block not the head
    return PTR_PLUS_BYTES(myHead, sizeof(el_blockhead_t));
  }
  else
  {
    //you still wanna do this stuff
    myHead->state = EL_USED;
    el_remove_block(el_ctl.avail, myHead);
    el_add_block_front(el_ctl.used, myHead);
    //returning the usable block not the head
    return PTR_PLUS_BYTES(myHead, sizeof(el_blockhead_t));
  }
}

////////////////////////////////////////////////////////////////////////////////
// De-allocation/free() related functions

// REQUIRED
// Attempt to merge the block lower with the next block in
// memory. Does nothing if lower is null or not EL_AVAILABLE and does
// nothing if the next higher block is null (because lower is the last
// block) or not EL_AVAILABLE.  Otherwise, locates the next block with
// el_block_above() and merges these two into a single block. Adjusts
// the fields of lower to incorporate the size of higher block and the
// reclaimed overhead. Adjusts footer of higher to indicate the two
// blocks are merged.  Removes both lower and higher from the
// available list and re-adds lower to the front of the available
// list.
void el_merge_block_with_above(el_blockhead_t *lower){
  //doing these checks to make sure I don't call my helper functions withh an invalid address
  if(lower == NULL || lower->state == EL_USED)
  {
    return;
  }
  //we know lower is a valid address so now we can access block above now safely
  el_blockhead_t *higher = el_block_above(lower);
  //temporarily remove lower from avail
  el_remove_block(el_ctl.avail, lower);
  if((higher != NULL))
  {
    if(higher->state == EL_AVAILABLE)
    {
      // we know we're merging so now we can remove higher from avail
      el_remove_block(el_ctl.avail, higher);
      //what the new size will be
      size_t sizeAddition = higher->size + EL_BLOCK_OVERHEAD;
      //updating size
      lower->size += sizeAddition;
      //updating size in the new foot
      el_blockfoot_t *highfoot = el_get_footer(higher);
      highfoot->size = lower->size;
    }
  }
  //we still wanna add this to the front
  el_add_block_front(el_ctl.avail, lower);
}

// REQUIRED
// Free the block pointed to by the give ptr.  The area immediately
// preceding the pointer should contain an el_blockhead_t with information
// on the block size. Attempts to merge the free'd block with adjacent
// blocks using el_merge_block_with_above().
void el_free(void *ptr){
  //getting the head of what we want to free
  el_blockhead_t *head = PTR_MINUS_BYTES(ptr, sizeof(el_blockhead_t));
  //getting the block below so we can also merge with below if possible
  el_blockhead_t *underhead = el_block_below(head);
  //making sure what we want to free isn't already free
  if(head->state == EL_AVAILABLE)
  {
    return;
  }
  //actually freeing the block
  el_remove_block(el_ctl.used, head );
  el_add_block_front(el_ctl.avail, head);
  //changing state
  head->state = EL_AVAILABLE;
  //trying to merge with both above and below
  el_merge_block_with_above(head);
  el_merge_block_with_above(underhead);
}
