#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
/*****************************************
 *         Linux Memory Page Map         *
 * Evan Schoenberger & Nathaniel Manning *
 *****************************************/

// PID of process we're mapping.
int pid = 0;
// Reference to /proc/<<pid>>
char procDir[256];
// Files to be found in /proc/<<pid>>
FILE *pagemap;
FILE *maps;

typedef struct components{
	uint64 page_frame_number[55];
	uint64 swap_type_if_swapped[5];
	uint64 swap_offset_if_swapped[49];
	uint64 pte[1];
	uint64 page_exclusively_mapped[1];
	uint64 zero[3];
	uint64 file_page_or_shared_anon[1];
	uint64 swapped[1];
	uint64 present[1];
};

typedef struct map_entry_t {
long addr_start
long addr_end
char[4] protection
long offset
char[5] stuff
long inode
char *program
map_entry_t *next;
};

void parseMaps(FILE* maps)
{
	
}

/**
 * [3] Finds pagemap and map references.
 * closes program with error if unable to locate.
 */
void findPidFiles()
{
	pagemap = fopen("pagemap", "r");
   maps = fopen("maps", "r");
	if (pagemap == NULL)
	{
		// TODO: add more specific error messages that have to do with
		// permissions.
		fprintf(stderr, "Error - pagemap not found\n");
	}
	if (maps == NULL)
	{
		fprintf(stderr, "Error - maps not found\n");
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
	// [1]
	if (argc >=2)
	{
		acceptInitialInput(argv[1]);
	}
	else
	{
		fprintf(stderr, "Error - No input provided\n");
		exit(0);
	}
	// [2]
	findProcDir();
	// [3]
   findPidFiles();
}
