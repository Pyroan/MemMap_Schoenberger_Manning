#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
/*****************************************
 *         Linux Memory Page Map         *
 *         Evan S. & Nathaniel M.        *
 *****************************************/

// PID of process we're mapping.
int pid = 0;
// Reference to /proc/<<pid>>
char procDir[256];
// Files to be found in /proc/<<pid>>
FILE *pagemap;
FILE *maps;

// Components of a pagemap entry
typedef struct components{
	uint64_t page_frame_number;
	uint64_t swap_type_if_swapped;
	uint64_t swap_offset_if_swapped;
	uint64_t swapped;
	uint64_t present;
} components_t;

components_t *component;

// Data of map entry
typedef struct map_entry {
uint64_t addr_start;	// beginning address
uint64_t addr_end;	// end address
char *protection;		// protection values
uint64_t offset;		// offset width
char *major_minor;			// we don't really know
uint64_t inode;		// node in the tree
char *program;			// name of program
struct map_entry *next;	// Next node in linked list.
} map_entry_t;

map_entry_t *head;

/*
 * [9] Parses info from page map file
 */
components_t* parsePageMap(uint64_t data)
{
	component->page_frame_number = data & 0x007fffffffffffff; 
	component->present = data >> 63;
	component->swapped =  (data >> 62)%2;
	component->swap_type_if_swapped = data & 0x00000000000001f;
	component->swap_offset_if_swapped = data & 0x007fffffffffffe0;

}
/*
 * [4] Tokenizes the map, stores entry  
 */
void parseMaps()
{
   char str[128];
	fgets(str, 128, maps);
	while(fgets(str, 128, maps) != NULL)
   {
		// Create a new entry
		map_entry_t *entry = malloc(sizeof(map_entry_t));
		char *tok = strtok(str, " -");
		// Add start and end addresses		
		entry->addr_start = strtol(tok, NULL, 16);
      tok = strtok(NULL, " ");
		entry->addr_end = strtol(tok, NULL, 16);
		// Add permissions
		tok = strtok(NULL, " ");
		entry->protection = malloc(sizeof(tok));
		strcpy(entry->protection, tok);
		// Add offset
		tok = strtok(NULL, " ");
		entry->offset = strtol(tok, NULL, 16);
		// Add major and minor
		tok = strtok(NULL, " ");
		entry->major_minor = malloc(sizeof(tok));
		strcpy(entry->major_minor,tok);
		// Add inode
		tok = strtok(NULL, " ");
		entry->inode = strtol(tok, NULL, 16);
		// Add program
		do
		{
			tok = strtok(NULL, " ");
		} while (tok == NULL);
		entry->program = malloc(sizeof(tok));
		strcpy(entry->program, tok);
		
		// Add new entry to linked list
		map_entry_t *current = head;
		while (current->next != NULL)
		{
			current = current->next;
		}
		current->next = entry;
   }
}

/**
 * [11] Shows total number of pages, number pages present, swapped, and
 * not present. Also print total MB for each section
 */
void printTotal(uint64_t total, char *name)
{
	double memUsed = total * getpagesize();
	memUsed /= 1000000; // convert bytes to MB
	printf("Total %s: %" PRIu64 " (%.1lfMB)\n", name, total, memUsed);
}

/**
 * [5] Converts an address to its corresponding
 * page number.
 */
uint64_t findPageNo(uint64_t addr) 
{
	int pageSize = getpagesize();
	return addr/pageSize;
}

/**
 * [3] Finds pagemap and map references.
 * closes program with error if unable to locate.
 */
void findPidFiles()
{
	// Open pagemap
	pagemap = fopen("pagemap", "r");
	// Check for errors with pagemap
	if (pagemap == NULL)
	{
		if (errno = ENOENT)
			fprintf(stderr, "Error - pagemap not found\n");
		else
			fprintf(stderr, "Error - unable to open pagemap\n");
	}
	// open maps
	maps = fopen("maps", "r");
	// Check for errors with maps
	if (maps == NULL)
	{
		if (errno = ENOENT)
			fprintf(stderr, "Error - maps not found\n");
		else
			fprintf(stderr, "Error - unable to open maps\n");
	}
}

/**
 * [2] Attempts to find corresponding directory in /proc.
 * closes program with error if unable to locate.
 */
void findProcDir()
{
	// Set name of dir we're looking for
	sprintf(procDir, "/proc/%d", pid);
	// Attempt to open the dir.
	// Check for errors
	if (chdir(procDir) == -1)
	{
		fprintf(stderr, "Error - %s not found\n", procDir);
		exit(0);
	}
}

/**
 * [1] Gets initial PID input and makes sure it's the proper input type.
 */
void acceptInitialInput(char *input)
{
   // Set pid
	pid = atoi(input);
	
	// If input was not a valid integer, return an error and exit.
	if (pid == 0)
	{
		fprintf(stderr, "Error - Invalid PID: %s\n", input);
		exit(0);
	}

}

/**
 * main
 */
int main(int argc, char **argv)
{
	head = malloc(sizeof(map_entry_t));
	component = malloc(sizeof(components_t));
	// [1] Take input, exit with error if no input provided.
	if (argc >=2)
	{
		acceptInitialInput(argv[1]);
	}
	else
	{
		fprintf(stderr, "Error - No input provided\n");
		exit(0);
	}
	// [2] Check if the /proc/<<pid>> directory exists
	findProcDir();
	// [3] open the pagemap and maps files in /proc/<<directory>>
   findPidFiles();
	// [4] parse the maps
   parseMaps();
   
   uint64_t totalPages = 0;
   uint64_t totalPresent = 0;
   uint64_t totalSwapped = 0;
   // For each entry we found in maps:
   map_entry_t *currentEntry = head;
   currentEntry = currentEntry->next;
   while (currentEntry != NULL)
   {
   	// [5] Find start and end pages numbers
   	uint64_t startPage = findPageNo(currentEntry->addr_start);
   	uint64_t endPage = findPageNo(currentEntry->addr_end);
   	
   	// [10] Print current entry data
   		printf("Segment %s\n", currentEntry->program);
   		printf("%s  ", currentEntry->protection);
   		printf("(%" PRIx64 " to %" PRIx64 ")\n", startPage, endPage);
   		printf("------------------------------\n");
   	
   	// [6, 7] Go through every page number in range of startPage to endPage...
   	for (uint64_t i = startPage; i <= endPage; i++)
   	{
   	   // ...find its corresponding entry in the pagemap file.
   		int fd = fileno(pagemap);
   		lseek64(fd,i*8,SEEK_SET);
   		uint64_t data;
   		read(fd, &data, 8);
   		// Parse Page data, store in component
   		parsePageMap(data);
   		// Increment totals accordingly
   		if(component->swapped)
   		{
   			totalSwapped++;
   		}
   		if(component->present)
   		{
   			totalPresent++;
   		}
   		
   		totalPages++;
			// [10] Print current Page data
			// Page number
			printf("%" PRIx64 "  ", i * getpagesize());
			// PFN
			printf("(%" PRIx64 "):  ", data);
			// ram or swap
			printf("[%s] ", component->swapped?"swap":"ram");
			if (component->swapped)
			{
				printf("(type=%" PRIx64 ",", component->swap_type_if_swapped);
				printf("(offset=%" PRIx64 ")\n", component->swap_offset_if_swapped);
			} else
			{
				printf("(shift=%d,", component->present?12:0);
				printf("swapped=%" PRIu64 ",", component->swapped);
				printf("present=%" PRIu64 ",", component->present);
				printf("pfn=%" PRIx64 ")", component->page_frame_number);
				printf("\n");
			}
   	}
   	currentEntry = currentEntry->next;
   }
   
   // [11] Output totals first: total number of pages.
   printf("------------------------\n");
	printTotal(totalPages, "pages");
	printTotal(totalPresent, "present");
	printTotal(totalSwapped, "swapped");
	printTotal(totalPages-totalPresent, "not present");	
	// Free memory
	fclose(maps);
	fclose(pagemap);
}
