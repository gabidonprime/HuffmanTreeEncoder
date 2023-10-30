// Interface to the File ADT

// !!! DO NOT MODIFY THIS FILE !!!

#ifndef FILE_H
#define FILE_H

#include <stdbool.h>

#define MAX_TOKEN_LEN 4

typedef struct file *File;

/**
 * Opens a file for reading
 * The file must be closed with FileClose
 */
File FileOpenToRead(char *filename);

/**
 * Opens a file for writing
 * The file must be closed with FileClose
 */
File FileOpenToWrite(char *filename);

/**
 * Closes the file
 */
void FileClose(File file);

/**
 * Reads a token from the file and stores it in the given array
 * Assumes that the file is open for reading
 * Returns true if a token was successfully read, and false otherwise
 */
bool FileReadToken(File file, char arr[]);

/**
 * Writes a string to the file
 * Assumes that the file is open for writing
 */
void FileWrite(File file, char *str);

#endif
