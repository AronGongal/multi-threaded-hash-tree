#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>     // for EINTR
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include "common.h"


// Print out the usage of the program and exit.
void Usage(char*);
uint32_t jenkins_one_at_a_time_hash(const uint8_t* , uint64_t );

void* tree_hash(void *arg);

#define BSIZE 4096
uint8_t* arr;

uint32_t mthreads;
uint32_t blocks_per_thread;
uint64_t bytes_per_thread;

int
main(int argc, char** argv)
{
  int32_t fd;
  uint32_t nblocks;

  // input checking
  if (argc != 3)
    Usage(argv[0]);

  // open input file
  fd = open(argv[1], O_RDWR);
  if (fd == -1) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  // use fstat to get file size
  // calculate nblocks
  struct stat buf;
  fstat(fd, &buf);
  
  nblocks = buf.st_size / BSIZE;
  mthreads = atoi(argv[2]);
  blocks_per_thread = nblocks/mthreads;
  bytes_per_thread = (uint64_t)blocks_per_thread * BSIZE;
  
  printf(" no. of blocks = %u \n", nblocks);

  arr = (uint8_t*)mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  
  uint32_t thread_num = 0;
  uint32_t hash;
  uint32_t* hash_ptr;
  pthread_t p1;

  double start = GetTime();

  // calculate hash value of the input file
  pthread_create(&p1, NULL, tree_hash, &thread_num);
  pthread_join(p1, (void**)&hash_ptr);
  hash = *hash_ptr;
  
  double end = GetTime();
  printf("hash value = %u \n", hash);
  printf("time taken = %f \n", (end - start));
  close(fd);
  return EXIT_SUCCESS;
}

uint32_t
jenkins_one_at_a_time_hash(const uint8_t* key, uint64_t length)
{
  uint64_t i = 0;
  uint32_t hash = 0;

  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}


void
Usage(char* s)
{
  fprintf(stderr, "Usage: %s filename num_threads \n", s);
  exit(EXIT_FAILURE);
}


void*
tree_hash(void* arg)
{
	uint32_t thread_num = *(uint32_t *)arg;
	uint32_t hash;
	uint64_t offset = thread_num * bytes_per_thread;
	
	// compute hash value of assigned blocks
	hash = jenkins_one_at_a_time_hash(&arr[offset], bytes_per_thread);
	
	uint32_t left_num = 2 * thread_num + 1;
	uint32_t right_num = left_num + 1;
	
	// interior thread
	if (left_num < mthreads) // has left child
	{
		uint8_t child_count;
		if (right_num < mthreads) // has right child
		{
			child_count = 2;
		}
		else
		{
			child_count = 1;
		}
		
		pthread_t p[2];
		uint32_t* h[2];
		uint32_t child_num[2] = {left_num, right_num};
		
		// create threads
		for (int i = 0; i < child_count; i++)
		{
			pthread_create(&p[i], NULL, tree_hash, &child_num[i]);
		}
		
		char total_hash_string[30];
		sprintf(total_hash_string, "%u", hash);
		
		// join threads and concatenate hash values of child threads
		for (int i = 0; i < child_count; i++)
		{
			h[i] = malloc(sizeof(uint32_t));
			pthread_join(p[i], (void**)&h[i]);
			
			char hash_str[11];
			sprintf(hash_str, "%u", *h[i]);
			free(h[i]);
			strcat(total_hash_string, hash_str);
		}
		
		// get hash value of concatenated string
		hash = jenkins_one_at_a_time_hash((uint8_t*)total_hash_string, strlen(total_hash_string));
	}
	uint32_t* hash_ptr = malloc(sizeof(uint32_t));
	*hash_ptr = hash;
	
	// return hash value
	pthread_exit((void*)hash_ptr);
}