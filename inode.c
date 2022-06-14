#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "blocks.h"
#include "inode.h"
#include "bitmap.h"

const int max_num = 32;

//get the inode the inum position
inode_t *get_inode(int inum) {
       assert(inum < max_num);
       inode_t *nodes = (inode_t*) blocks_get_block(0);
       return &(nodes[inum]);
}

//allocate a new inode
int alloc_inode() {
	for (int i = 0; i < max_num; ++i) {
        inode_t* node = get_inode(i);
        if (node->mode == 0) {
            memset(node, 0, sizeof(inode_t));
            node->refs = 0;
            node->mode = 010644;
            node->size = 0;
	    node->block = alloc_block();
            return i;
        }
    }

    return -1;
}

//make the inode grow
int grow_inode(inode_t *node, int size){
	node->size = size;
	return 0;

}

//make the inode shrint
int shrink_inode(inode_t *node, int size){
       node->size = size;
       return 0;
}


