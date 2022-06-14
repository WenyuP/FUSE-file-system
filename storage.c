
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include <libgen.h>
#include <bsd/string.h>
#include <stdint.h>
#include <stdlib.h>

#include "storage.h"
#include "slist.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"
#include "bitmap.h"

//search for a file in the storage by its path
int storage_lookup(const char *path) {

    int inum = tree_lookup(path);

    if (inum == -1) {
        return -1;
    }

    inode_t *node = get_inode(inum);
    printf("storage lookup:%d",inum);
    return 0;
}

//save the information of an inode in its stat
int storage_stat(const char *path, struct stat *st) {

   int inum = tree_lookup(path);

   if (inum < 0) {
      printf("No specific node: in storage_stat: %s\n",path);	   
      
      return inum;
   }

   if (inum >= 0) {
   inode_t *node = get_inode(inum);

   st->st_mode = node->mode;

   st->st_size = node->size;

   st->st_uid = getuid();

   printf("storage_stat: path %s, mode %d, inum, %d \n",path, node->mode, inum);
   return 0;
}
}

//initialize a storage
void storage_init(const char* path) {

	blocks_init(path);

	directory_init();
}

//list all the files in the given path
slist_t* storage_list(const char* path) {

	printf("storage_list: path %s\n",path);

	return directory_list(path);
}


//make a new inode
int storage_mknod(const char *path, int mode) {
  
	int temp = tree_lookup(path);

	if (temp > -1) {
		printf("The node already exists in the system");
		 return -EEXIST;
    }

//build the filepath for the new node
char* parent = (char*)malloc(strlen(path));

char* child = (char*)malloc(strlen(path));

strcpy (child,path);

strcpy (parent,path);

child = strchr(child, '/');

child += 1;

int difference = strlen(path) - strlen(child);

parent[difference] = 0;

int parent_inum = tree_lookup(parent);

inode_t* parent_inode = get_inode(parent_inum);

int new_node_inum = alloc_inode();

if (new_node_inum > -1) {

	inode_t* new_node = get_inode(new_node_inum);

	new_node->mode = mode;

	new_node->size = 0;

	new_node->refs = 1;
        
	int r = alloc_block();
        
	if (r > -1) {
	new_node->block = alloc_block();
       	
	int rv = directory_put(parent_inode,child,new_node_inum);
}

        printf("New node of size: %d\n",new_node->size);
        free(parent);

        return 0 ;
}
}

//read from the path
int storage_read(const char *path, char *buf, size_t size, off_t offset) {

	int inum = tree_lookup(path);

	if (inum < 0) {
		return inum;
	}

	inode_t* curr = get_inode(inum);

	printf("+ storage_read(%s); inode %d\n", path, inum);

	if (curr->size <= offset) {
                return 0;
        }

	//
	if (size + offset > curr->size) {
		size = curr->size - offset;
	}

	char* temp = (char*)blocks_get_block(curr->block);
	memcpy(buf,temp + offset, size);
	return size;
}

//truncate the storage
int storage_truncate(const char *path, off_t size) {
	int inum = tree_lookup(path);

	if (inum < 0) {
		return inum;
	}

	int rv = 0;

	inode_t *n = get_inode(inum);

	if (size < n->size) {
		int rv = shrink_inode(n,size);
	}

	if (size >= n->size) {
		int rv = grow_inode(n,size);
	}
	return rv;
}

//write from the buffer to the path
int storage_write(const char* path, const char* buf, size_t size, off_t offset){
	int t = storage_truncate(path, offset + size);

        if (t < 0) {
                printf("fail to truncate");
                return t;
        }
	int inum = tree_lookup(path);

	if (inum < 0) {
		printf("storage_write: inum < 0\n");
		return inum;
	}

	inode_t* node = get_inode(inum);

	printf("+ storage_write(%s); inode %d\n", path, inum);

	char* temp = (char*)blocks_get_block(node->block);

	if (offset >= node->size) {
		printf("offset larger than size,%d",node->size);
		return -1;
	}

	if (offset + size > 4096) {
		size = 4096 - offset;
		printf("offset + size > 4096, change the size\n");
	}

	memcpy(temp + offset, buf,size);
	return size;
}

//linking function for storage
int storage_link(const char *from, const char *to){

        int inum = tree_lookup(from);

    char* name = (char*)malloc(strlen(to));
    char* parent = (char*)malloc(strlen(to));
    char* temp = name;

    strcpy(name, to);
    strcpy(parent, to);
    name = strrchr(name, '/') + 1;
    //

    int d = strlen(to) - strlen(name);
    parent[d] = 0;

    int parent_inode = tree_lookup(parent);
     printf("p_in: %d\n",parent_inode);
     printf("inum: %d\n",inum);

        if (inum < 0) {
		return inum;
	}

	if (parent_inode < 0) {
		return parent_inode;
	}
    
        inode_t* in = get_inode(inum);
    in->refs++;
    
    inode_t* pnode = get_inode(parent_inode);

    directory_put(pnode, name, inum);
    free(temp);
    free(parent);
    return 0;
}

//the unlinking process 
int storage_unlink(const char* path){
    char* name = (char*)malloc(strlen(path));
    
    char* parent = (char*)malloc(strlen(path));

    char* temp = name;

    strcpy(name, path);

    strcpy(parent, path);

    name = strrchr(name, '/') + 1;

  
    int d = strlen(path) - strlen(name);

    parent[d] = 0;

    int parent_inode = tree_lookup(parent);
    int inum = tree_lookup(path);

     if (inum < 0) {
                return inum;
        }

     if (parent_inode < 0) {
                return parent_inode;
        }

    int rv = 0;

    inode_t* in = get_inode(inum);

    inode_t* p_node = get_inode(parent_inode);

    if (in->refs > 1) {

        in->refs--;
    } else {
        
        free_block(in->refs);

        void* bit_m = get_inode_bitmap();
        bitmap_put(bit_m,inum, 0);
    }

    rv = directory_delete(p_node, name);
    free(temp);
    free(parent);
    return rv;
             
}
//renaming process
int storage_rename(const char* from, const char* to){
    int inum = tree_lookup(from);
	if (inum < 0){
		return inum;
	}

	int rv = storage_link(from,to);
	if (rv < 0){
		return rv;
	}

	return storage_unlink(from);
}
