// C program mygrep.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MAX_LINE_LENGTH 512

//Prototypes:
char *read_line(FILE *fpntr);
int grep_stream(FILE *fpntr, char *string, int iflag, int nflag, int vflag);

// -v: Lines that DO NOT match pattern are printed. (So strstr() == NULL)
// -i: Ignore the case of the pattern in files (strcasestr()). Although that is non-standard so may need another way.
// -n: Print the line number that the line occurs on (nl).

// You should also be able to accept any combination of the arguments (i.e -vin, -niv, -inv).

// You should also be able to accept any number of files to run grep_stream on as well.

int line_number = 1;		 	 // Line number in the file.
int width = 6;					 // Spacing

int main(int argc, char *argv[])
{
	//fprintf(stdout, "Initialize flags and variables.\n");
	
	FILE *fpntr = NULL;			   	 // Pointer
	char *PATTERN = argv[1]; 	     // String we are attempting to match with our search.
	char *FILE_PATH = NULL;       	 // Optional file pathway.
	int eflag = 0; 				  	 // Track errors
	int vflag = 0;					 // Track v-param usage
	int iflag = 0;					 // Track i-param usage
	int nflag = 0;					 // Track n-param usage
	
	int index;
	int file_index_offset = 1;
	//printf("Program Logic Begins.\n");
	
	if(argc > 2)
	{
		// Parse the entire command.
		for(index = 2; index < argc; index++)
		{
			//printf("Inside the for-loop\n");
			
			// Look at parameters preceeded by a double hypen (--).
			if(strncmp(argv[index], "--", 2) == 0)
			{
				//printf("Inside the -- conditional\n");
				
				if(strncmp(argv[index], "--line-number", 13) == 0)
					nflag = 1; 
				
				else if(strncmp(argv[index], "--ignore-case", 13) == 0)
					iflag = 1;
				
				else if(strncmp(argv[index], "--invert-match", 14) == 0)
					vflag = 1;
				
				// Usage error.
				else
				{
					eflag = 1;
					fprintf(stderr, "Proper usage of the program is mygrep \"STRING\" [FILENAME]\n");
					exit(EXIT_FAILURE);
				}
			}
			
			// Look at parameters preceeded by a hypen (-).
			else if(strncmp(argv[index], "-", 1) == 0)
			{
				//printf("Inside the - conditional\n");
				
				// Case: -n[vi] or -n[iv]
				if(argv[index][1] == 'n')
				{
					nflag = 1;
					int i;
					
					for(i = 2; i < strlen(argv[index]); i++)
					{
						if(argv[index][i] == 'v')
							vflag = 1;
						
						else if(argv[index][i] == 'i')
							iflag = 1;
					}
				}
				
				// Case: -v[ni] or -v[in]
				else if(argv[index][1] == 'v')
				{
					vflag = 1;
					int i;
					
					for(i = 2; i < strlen(argv[index]); i++)
					{
						if(argv[index][i] == 'n')
							nflag = 1;
						
						else if(argv[index][i] == 'i')
							iflag = 1;
					}
				}
				
				// Case: -i[nv] or -i[vn]
				else if(argv[index][1] == 'i')
				{
					iflag = 1;
					int i;
					
					for(i = 2; i < strlen(argv[index]); i++)
					{
						if(argv[index][i] == 'n')
							nflag = 1;
						
						else if(argv[index][i] == 'v')
							vflag = 1;
					}
				}
				
				// Usage error.
				else
				{
					eflag = 1;
					fprintf(stderr, "Proper usage of the program is mygrep \"STRING\" [FILENAME]\n");
					exit(EXIT_FAILURE);
				}
			}
			
			file_index_offset++;
		}
	}
	
	printf("FIO: %d\n", file_index_offset);
	printf("Argc: %d\n\n", argc);
	
	// There are no files.
	if(file_index_offset == argc)
	{
		line_number = 1;
		//printf("Inside the stdin conditional\n");
		fpntr = stdin;										// Pointer to stdin
		grep_stream(fpntr, PATTERN, iflag, nflag, vflag);	// mygrep the file
		fclose(fpntr);										// Close the file.
	}
	
	// There is at least one file.
	else
	{
		int i = 0;
		for(i = file_index_offset; i < argc; i++)
		{
			FILE_PATH = argv[i];			// Set the file path.
			line_number = 1;				// Need to reset the line number for each new file.
			fpntr = fopen(FILE_PATH, "r");  // Open file to read
			
			// Make sure the file and pointer are valid.
			if(fpntr != NULL && FILE_PATH != NULL)
			{
				grep_stream(fpntr, PATTERN, iflag, nflag, vflag);
				fclose(fpntr);
			}
			
			// The file doesn't exist.
			else
			{
				eflag = 1;
				fprintf(stderr,"mygrep: %s: No such file or directory\n", FILE_PATH);
				exit(EXIT_FAILURE);
			}
		}
	}
	
	//There were no errors present, program ran successfully.
	if(eflag == 0)
		return EXIT_SUCCESS;
	
	//There were errors present, the program did not run successfully.
	else if(eflag == 1) 
		return EXIT_FAILURE;
}

//Function to get next line from an open file/stdin.
//Returns pointer to line buffer.
char *read_line(FILE *fpntr)
{
	static char line_buff[MAX_LINE_LENGTH];
	int i = 0;
	int next = NULL;
	
	// Read the file until EOF or next line character is encountered.
	while((next = fgetc(fpntr)) != '\n' || next == EOF)
	{
		// We have reached the end of the file.
		if(next == EOF)
			return NULL;
		
		// Move the pointer.
		line_buff[i++] = next;
		line_buff[i] = '\n';
	}
	
	line_buff[i] = '\0';						// Null terminate the line.
	realloc(line_buff[i], sizeof line_buff);	// Reduce the size of the return buffer.
	line_number++;								// Move to the next line.
	
	return line_buff;
}

//Parses a character buffer searching for a pattern within that buffer 
//returning either 1 for at least one successful match or 0 for none.
int grep_stream(FILE *fpntr, char *string, int iflag, int nflag, int vflag) 
{
	int foundString = 0;
	char *line;
	
	//While the file contains data and the next line isn't null (aka end of file).
	while((line = read_line(fpntr)) != NULL)
	{
		//printf("Inside the while conditional\n");
		// There are no parameters.
		if(iflag + nflag + vflag == 0)
		{
			//printf("Inside the no params conditional\n");
			if(strstr(line, string) != NULL)
			{
				fprintf(stdout, "%s\n", line);
				foundString++;
			}
		}
		
		// There is only 1 parameter.
		else if(iflag + nflag + vflag == 1)
		{
			//printf("Inside the 1 param conditional\n");
			// Ignore case.
			if(iflag == 1)
			{
				if(strcasestr(line, string) != NULL)
				{
					fprintf(stdout, "%s\n", line);
					foundString++;
				}
			}
			
			// Print the line numbers.
			else if(nflag == 1)
			{
				if(strstr(line, string) != NULL)
				{
					fprintf(stdout, "%*d:%s\n", width, line_number, line);
					foundString++;
				}
			}
			
			// Print lines that DO NOT match the pattern.
			else if(vflag == 1)
			{
				if(strstr(line, string) == NULL)
				{
					fprintf(stdout, "%s\n", line);
					foundString++;
				}
			}
		}
	
		// There are 2 parameters
		else if(iflag + nflag + vflag == 2)
		{
			//printf("Inside the 2 params conditional\n");
			// Ignore case and print the line with numbers.
			if(iflag == 1 && nflag == 1)
			{
				if(strcasestr(line, string) != NULL)
				{
					fprintf(stdout, "%*d:%s\n", width, line_number, line);
					foundString++;
				}
			}
			
			// Ignore case and print the lines that DO NOT match the pattern.
			else if(iflag == 1 && vflag == 1)
			{
				if(strcasestr(line, string) != NULL)
				{
					fprintf(stdout, "%s\n", line);
					foundString++;
				}
			}
			
			// Print the line numbers of lines that DO NO match the pattern.
			else if(nflag == 1 && vflag == 1)
			{
				if(strstr(line, string) == NULL)
				{
					fprintf(stdout, "%*d:%s\n", width, line_number, line);
					foundString++;
				}
			}
		}
		
		// There are 3 parameters. (THIS IS WRONG).
		else if(iflag + nflag + vflag == 3)
		{
			//printf("Inside the 3 params conditional\n");
			
			if(strcasestr(line, string) == NULL)
			{
				fprintf(stdout, "%*d:%s\n", width, line_number, line);
				foundString++;
			}
		}
	}	
	
	//We found a match, so we return success.
	if(foundString != 0)
		return 1;
	
	//We did not find a match.
	else
		return 0;
}
// EOF
