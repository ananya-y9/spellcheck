This program serves as a spell checker for text files, aiming to identify misspelled
words by comparing them against a provided dictionary. It traverses through directories, reads
text files, and checks each word against the dictionary. If a word is not found in the dictionary, it
is considered misspelled and reported along with its line and column number.

Requirements:
This program requires a dictionary file containing correctly spelled words.
Text files to be spell-checked are provided as input arguments to the program.
The program must be compiled with the arraylist.c and arraylist.h files.


Design Notes:
Our spchk.c file:
- We chose to store the words read off the dictionary into a dynamic array list structure (arraylist.c and
arraylist.h) to ensure efficient searching and retrieval. once words are retrieved from the dictionary, they will be pushed onto the
array list and sorted.
- Our program  also utilizes a binary search algorithm to efficiently search for words in the dictionary, after the
dictionary has been sorted using the C standard library function qsort().
- Words with capitalization variations (e.g., capitalized at the beginning) are considered separate words in the dictionary.
(each capitalization variation of a word read off from the dictionary is also pushed into the arraylist).
- Our traverse_dir() function is responsible for recursively traversing directories, identifying text files, and reading their contents
for spell-checking. It will open a specified directory using opendir and then iterate through each entry in the directory. if the entry
is a subdirectory, recursively call traverse_dir on it. if the entry is a regular file and has a .txt extension, open it for reading.
for each text file we find, we call read_lines to process the text for spell-checking. we close the directory after going
through all of its entries. we do not open any files that start with "." (hidden files within a directory)
- We implemented a read_lines() function that reads data from the file descriptor fd in chunks. it iterates through the chunks and
processes each line. when a complete line is found, it calls the use_line function, passing the processed line and the provided argument
arg. it repeats this process until the entire file is read. the purpose of this is to facilitate reading lines from a text file more
efficiently, handling large files by reading them in smaller manageable chunks.
- Our search_words() function is called for each line of text read by the read_lines function. It breaks down a line into individual
words, calls functions taking care of punctuation and hyphens, and searches for each word in a dictionary using binary search.
If a word is not found in the dictionary after binary search, it is considered misspelled.
- Words are sanitized by removing/ignoring punctuation at the end or beginning of each word, before being
checked against words the dictionary. Correctly spelled words regardless of punctuation (at the beginning or end)
are correct, however, a word with bad spelling will be reported as an error. Words containing punctuation in the
middle will be reported as a spelling error.
- Additionally, our program identifies all sorts of hyphenated words: it checks each part of the word seperately,
searches for it in the dictionary and then reports an error in any part of the hyphenated word.
- Memory management is handled by dynamically allocating and freeing memory as needed.
- Our program returns EXIT_SUCCESS (0) if all words in the provided text files are correctly spelled. if one or more misspelled
words are found, the program returns EXIT_FAILURE (1). To keep track of misspelled words, we used a global variable "wrongwords".
- Debugging statements are included but commented out. They can be enabled by setting the DEBUG macro to 1.
- after this process is done, our main function will destroy dictionary and free all allocated memory.


*Our test scenarios and test cases:
Testing with a single text file.
Testing with multiple text files in a directory.
Testing with multiples directories in a directory.
Testing for words with hyphens. (with one of the words misspelled/both words misspelled/both correctly spelled) (as well as with
differentspacing between the words and the hyphens)
 Testing for words with capitalization variations.
Testing for words containing punctuation in the middle as well as the beginning or the end.
Testing for numbers as well as for words containing numbers and special characters.
Testing for hidden files in adirectory but also as a singular text file.

Instructions for Running:
Compile the program along with arraylist.c
Run the compiled executable providing the path to the dictionary file as the first argument,
followed by the paths to the text files or the directories to traverso through to be spell-checked.
