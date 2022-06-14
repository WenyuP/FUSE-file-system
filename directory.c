#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "directory.h"
#include "inode.h"
#include "blocks.h"

//search for a certain node with a given path in the directory
int directory_lookup(inode_t *dd, const char *name) {

	dirent_t* data = (dirent_t*) blocks_get_block(dd->block);

	if (strcmp(name, "") == 0) {
		return 0;
	}

	for (int i = 0; i < (4096 / sizeof(dirent_t)); ++i) {
		if (strcmp(name,data[i].name) == 0) {

			return data[i].inum;
		}
	}

	return -ENOENT;
}

//search for a file by its path
int tree_lookup(const char* path)
{
    assert(path[0] == '/');

    if (strcmp(path, "/") == 0) {
        return 0;
    }

     slist_t* path_temp = s_explode(path, '/');

        int rv = 0;

        while(path_temp != NULL) {

                inode_t* parent = get_inode(rv);

                char *temp = path_temp->data;

		rv = directory_lookup(parent, temp);

		path_temp = path_temp->next;
	}
	s_free(path_temp);
	
	return rv;
}

//initialize a directory
void directory_init() {

	int inum = alloc_inode();

	assert (inum > -1);

	inode_t* curr = get_inode(inum);

	curr->mode = 040755;

	curr->refs = 1;

	curr->size = 0;

	curr->block = alloc_block();


}
 
//put a file at inum position onto the directory
int directory_put(inode_t *dd, const char *name, int inum) {

dirent_t* list = (dirent_t*) blocks_get_block(dd->block);	

for (int i = 0; i < (4096/sizeof(dirent_t)); ++ i) {

	if(strcmp(list[i].name, "") == 0){

			strcpy(list[i].name, name);

			list[i].inum = inum;

			return 0;
		}
		
	}

	return -ENOENT;
}

//return a directory items as an slist
slist_t* directory_list(const char* path) {
        int inum = tree_lookup(path);

        inode_t* curr  = get_inode(inum);

        int temp = curr->block;

        dirent_t* dir = (dirent_t*) blocks_get_block(temp);

        slist_t* temp_list = NULL;

        int count = 0;
        for (int i = 0; i < (4096 / sizeof (dirent_t)); ++ i) {

                if (strcmp(dir[i].name,"") != 0) {
                        char* name = dir[i].name;

                        //printf("dir_name: %s\n",name);

                        temp_list = s_cons(name,temp_list);
                        count ++;
                }

        }

 
        return temp_list;
}

//remove an inode from a directory
int directory_delete(inode_t *dd, const char *name) {

	dirent_t* blocks = (dirent_t*)blocks_get_block(dd->block);

	for (int i = 0; i < (4096 / sizeof(dirent_t)); ++ i) {

		if (strcmp(blocks[i].name,name) == 0) {

			blocks[i].name[0] = 0;

			blocks[i].inum = 0;

			return 0;
		
		}
	}
	return -ENOENT;
}

