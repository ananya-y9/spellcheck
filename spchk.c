#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include "arraylist.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef BUFLENGTH
#define BUFLENGTH 16
#endif

#ifndef INITSIZE
#define INITSIZE 4
#endif

static arraylist_t dict;
static int wrongwords; //global variable that decides whether we system exit or not

char* binarysearch(arraylist_t *, char*);

void read_lines(int fd, void (*use_line)(void *, char *, char*), void *arg, char*filename)
{
    int buflength = BUFLENGTH;
    char *buf = malloc(BUFLENGTH);
    int pos = 0;
    int bytes;
    int line_start;
    
    while ((bytes = read(fd, buf + pos, buflength - pos)) > 0) 
    {
        if (DEBUG) {printf("read %d bytes; pos=%d\n", bytes, pos); }
        line_start = 0;
        int bufend = pos + bytes;

        while (pos < bufend) 
        {
            if (buf[pos] == '\n') 
            {
                // we found a line, starting at line_start ending before pos
                buf[pos] = '\0';
                use_line(arg, buf + line_start, filename);
                line_start = pos + 1;
            }
            pos++;
        }

        if (line_start == pos) 
        {
            pos = 0; // move segment to start of buffer and refill remaining buffer
        }
        else if (line_start > 0) 
        {
            int segment_length = pos - line_start;
            memmove(buf, buf + line_start, segment_length);
            pos = segment_length;
            //if (DEBUG) printf("move %d bytes to buffer start\n", segment_length);
        } 
        else if (bufend == buflength) 
        {
            buflength *= 2;
            buf = realloc(buf, buflength);
            if (DEBUG) printf("increase buffer to %d bytes\n", buflength);
        }
    }

    if (pos > 0) 
    {
        if (pos == buflength) 
        {
            buf = realloc(buf, buflength + 1);
        }
        buf[pos] = '\0';
        use_line(arg, buf + line_start, filename);
    }
    free(buf);
}

void print_line(void *st, char *line, char*filename)
{
    int *p = st;
    printf("%d: %s\n", *p+1, line);
    (*p)++;
}

int ignore_punc(char *word, char *newword)
{
    int count = 0; 
    //int k = 0;
    int start = -1; 
    int end = -1; 
    

    for(int i = 0; word[i] != '\0'; i++)
    {
        if (start == -1)
        {   
            if (((word[i] == '"') || (word[i] == '\'') || (word[i] == '(') || (word[i] == '[') || (word[i] == '{'))) //starting a word so ignore
            {
                count++;
                continue;
            }
            else
            {
                start = i;
            }
        }
        else
        {
            if (isalpha(word[i]) || !(ispunct(word[i]))) //wrong if not ending with a char or punctuation
            {
                end = i; //ignore punctuation at the end
            }
        } 
        /*
        
        if(isalpha(word[i]) && (start==-1)) //starts with a 
        {
            start = i; 
        }
        else if (isalpha(word[i]))
        {
            end = i;
        }

        if((!isalpha(word[i])) && ((i == 0) || (word[i+1] == '\0')))
        {
            continue; //ignore punctuation at the front or after a word
            //col++; 
        }
        else if((isalpha(word[i])) || (word[i]=='-') || (word[i]== '\'')) //not a valid punctuation: only valid ones are hyphen and apostrophe
        {
            newword[k++] = word[i];
        }*/
    }
    int m = 0; 
    for (int k = start; k <= end; k++)
    {
        newword[m] = word[k]; //copy into new word 
        m++;
    }
    newword[m] = '\0';

    return count;
}

int hyphen(char *word)
{
    int k = 0;
    for(int i = 0; word[i] != '\0'; i++)
    {
        if((word[i]=='-')) //not a valid punctuation: only valid ones are hyphen and apostrophe
        {
            return 1;  
        }
    }
    return k; 
}

int noletters(char *word)
{
    int t = 0; 
    for(int i = 0; word[i] != '\0'; i++)
    {
        if((isalpha(word[i])))//found a letter
        {
            return t;
        }
    }
    return 1; 
}

void search_words(void *st, char *line, char*filename)
{
    int *p = st;
    int col = 1; 
    char *linecopy = line;
    char *reset = strtok(linecopy, " "); //break up wherever see a space
    int hyphenval; 

    while (reset !=  NULL)
    {
        char neww[strlen(reset)+1];
        char *word;
        
        int count = ignore_punc(reset, neww); //remove punctuation from words
        char newwcopy[strlen(neww)+1];
        strcpy(newwcopy, neww); //make a new copy
        hyphenval = hyphen(neww);

        if(hyphenval)
        {
            for (int j = 0; newwcopy[j]!= '\0'; j++)
            {
                if (newwcopy[j] == '-')
                {
                    int incorrect = 0;

                    char firstword[j+1];
                    strncpy(firstword, newwcopy, j);
                    firstword[j] = '\0';
                    
                    word = binarysearch(&dict, firstword);
                    if (word == NULL)
                    {
                        incorrect = 1; 
                    }

                    char secondword[strlen(newwcopy)-j];
                    strcpy(secondword, newwcopy + j + 1);
                    word = binarysearch(&dict, secondword);
                    if (incorrect == 1)
                    {
                        word = NULL;
                    }
                }
            }
        }
        else
        {
            word = binarysearch(&dict, neww); //search for words in dictionary
        }

        if (word == NULL && (!noletters(neww))) //mispelled word and not punctuation
        {
            wrongwords = 1; 
            printf("%s (%d,%d): %s\n", filename, *p+1, col+count, neww); //print out the mispelled word and line and column
        }

        col = col+strlen(reset)+1; //increment column count by char
        reset = strtok(NULL, " ");
    }
    (*p)++;
}

void add_word(void *st, char *word, char*filename)
{
    al_push(&dict, word);
    //add up to 3 versions of the word
    char *mword1 = (char*)malloc(strlen(word)+1);
    char *mword2 = (char*)malloc(strlen(word)+1);
    memcpy(mword1, word, strlen(word)+1);
    memcpy(mword2, word, strlen(word)+1);

    if(islower(mword1[0])) //for the first letter to be capitalized
    {
        mword1[0] = toupper(mword1[0]);
        //if(DEBUG)printf("first capital: %s\n", mword1);
        al_push(&dict, mword1);
    }

    for (int i = 0; i < strlen(mword2);i++)
    {
        if(islower(mword2[i]))
        {
            mword2[i] = toupper(mword2[i]);
        }
    }

    if ((strcmp(mword2, mword1) != 0) && (strcmp(mword2, word) != 0)) //is different, not adding the same word
    {
        //if(DEBUG)printf("all capital: %s\n", mword2);
        al_push(&dict, mword2);
    }

    free(mword1);
    free(mword2);
}

void traverse_dir(char *filename)
{
    DIR *handle = opendir(filename);
    struct dirent *iterator;

    if (handle == NULL)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    struct stat buf;
    char name[2048];
    
    while ((iterator = readdir(handle))!= NULL) //files to traverse through
    {
        if (strcmp(iterator->d_name, ".") == 0 || strcmp(iterator->d_name, "..") == 0)
        {
            continue; //skip through this file bc it's parent and current directory
        }
            
        strcpy(name, filename);
        strcat(name, "/");
        strcat(name, iterator->d_name); //make path name
        if (DEBUG) printf("pathname: %s\n", name);

        int r = stat(name, &buf);
        if (r < 0) //file can't be open
        {
            printf("Failed to open!\n");
            exit(EXIT_FAILURE); 
        }

        if (S_ISDIR(buf.st_mode))
        {
            if (DEBUG) printf("directory name: %s\n", iterator->d_name);
            traverse_dir(name);
        }
        else
        {
            char *filen = iterator->d_name;
            if (((strlen(filen) > 4) && (strcmp(filen + strlen(filen) - 4, ".txt") == 0)) && (*filen != '.'))
            {
                int ft = open(name, O_RDONLY);
                if (ft < 0) 
                {
                perror(name);
                exit(EXIT_FAILURE);
                }
                int n = 0;
                
                if (DEBUG) printf("file name: %s\n", iterator->d_name); //ends in .txt and not a hidden directory/file
                read_lines(ft, search_words, &n, name);
            }
        }
    }
    closedir(handle);
}

int compare_strings(const void *a, const void *b) 
{
    return strcmp(*(const char **)a, *(const char **)b);
}

char* binarysearch(arraylist_t *dict, char *wordsearch)
{
    int size = al_length(dict);
    int low=0;
    int high = size-1;
    int mid;
    int cmp;

    while(low <= high){
        mid = (low+high)/2;
        cmp = strcmp((al_get(dict, mid)), wordsearch); // compare the dictionary arraylist at index middle withe word you're lookingfor
        if (cmp ==0)
        {
            return al_get(dict, mid); //hit!! found

        }
        else if ( cmp < 0)
        {
            low = mid +1; // look in upper half
        } else
        {
            high = mid -1; // look in lower half
        }
    }
    
    //out of loop means something is mispelled, compare to closest found thing
    return NULL; // if made it out of loop word not found
}

int main(int argc, char **argv) // TO RUN ./spchk ../dict ../testfile
{
    if (argc < 3)
    {
        printf("you forgot some arguments\n");
    }

    wrongwords = 0;

    char *dictfile = argv[1]; //dictionary is always going to be first argument "second" bc have to write executable
    struct stat buf;
    int r = stat(dictfile, &buf);
    if (r < 0) 
    {
        printf("Failed to open!\n");
        return -1; 
    }
    if (S_ISREG(buf.st_mode)) //is a file (dictionary)
    {
        al_init(&dict, INITSIZE);
        int fd = open(dictfile, O_RDONLY);
        if (fd < 0) 
        {
            perror(dictfile);
            exit(EXIT_FAILURE);
        }
        int n = 0;
        read_lines(fd, add_word, &n, dictfile);
        //al_print(&dict);
        if (DEBUG) printf("list length: %u\n", al_length(&dict));
        close(fd);
    } 
    else 
    {
        printf("wierd\n");
        return -1; 
    }

    if (DEBUG) //test to see if binary search works
    {
        char* found = binarysearch(&dict, "YOU");
        if(DEBUG)printf("found: %s\n", found); 
    }

    qsort(dict.data, dict.length, sizeof(char *), compare_strings); //sort for binary search

    //testfiles directories
    struct stat tbuf;
    for (int i = 2; i < argc; i++)
    {
        char *testfiles = argv[i];
        int rt = stat(testfiles, &tbuf);
        if (rt < 0) 
        {
            printf("Failed to open!\n");
            return -1; 
        }
        if (S_ISDIR(tbuf.st_mode))
        {
            traverse_dir(testfiles);
        } 
        else if (S_ISREG(tbuf.st_mode))
        {
                if (DEBUG) printf("run on single file\n");
                if (strcmp(testfiles, ".") == 0 || strcmp(testfiles, "..") == 0)
                {
                    break; //this is a directory, not a file
                }
            
                int ft = open(testfiles, O_RDONLY);
                if (ft < 0) 
                {
                    perror(testfiles);
                    exit(EXIT_FAILURE);
                }
                int n = 0;
                if (DEBUG) printf("file name: %s\n", testfiles); //ends in .txt and not a hidden directory/file
                read_lines(ft, search_words, &n, testfiles); 
        }
        else
        {
            printf("wierd\n");
            return -1; 
        }
    }

    al_destroy(&dict); //destroy the dictionary!!!

    if(wrongwords == 1) //there's a mispelled word, so do exit failure
    {
        return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS; //otherwise, everything spelled right!
} 