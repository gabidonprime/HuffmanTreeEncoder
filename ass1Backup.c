// COMP 2521 Assignment 1
// Huffman encoding and decoding file
// Written by Gabriel Esquivel (z5358503) 

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Counter.h"
#include "File.h"
#include "huffman.h"

// Structs definition
struct huffmanEncodedData {
    char letter;
    char *encoding;
};

// Helper Functions
struct huffmanTree *createHuffmanTreeNode(char *token, int frequency);
int compareHuffmanTreeNodesByFrequency(const void *a, const void *b);

struct huffmanEncodedData *encodeData(struct huffmanTree *root, char *input);
void populateHuffmanEncodeData(struct huffmanTree *tree, char *encoding, struct huffmanEncodedData *encodedData, int *index);
char *encodeText(struct huffmanEncodedData *encodedData, char *inputText, char *outputText);

char *FileToString(File file);
void findUniqueLetters(const char *input, char *uniqueLetters);
void appendString(char **output, char c, int *index);

// Task 1
// Decode the encoded text using the huffman tree
void decode(struct huffmanTree *tree, char *encoding, char *outputFilename) {
	// Setup output file
	struct file *outputFile = FileOpenToWrite(outputFilename);
    // Traverse the Huffman tree while decoding the text
	struct huffmanTree *root = tree;
    for (int i = 0; encoding[i] != '\0'; i++) {
        if (encoding[i] == '0') {
            tree = tree->left;
        } else if (encoding[i] == '1') {
            tree = tree->right;
        }
		// If leaf node, add character to output file
        if (tree->left == NULL && tree->right == NULL) {
            FileWrite(outputFile, tree->token);
            tree = root;
        }
    }
    FileClose(outputFile);
}

// Task 3
// Create a huffman tree from the input file
struct huffmanTree *createHuffmanTree(char *inputFilename) {
    // Read tokens from the input file and count token frequencies
    struct file *inputFile = FileOpenToRead(inputFilename);
    struct counter *c = CounterNew();
    char token[MAX_TOKEN_LEN + 1];
    while (FileReadToken(inputFile, token)) {
        if (isalpha(token[0]) || token[0] == ' ') {
            CounterAdd(c, token);
        }
    }
    FileClose(inputFile);

    // Create leaf nodes for each token with its frequency
    int numItems;
    struct item *items = CounterItems(c, &numItems);    // 10 items in the array

    // Allocate memory for nodes
    struct huffmanTree **nodes = (struct huffmanTree **)malloc((numItems) * sizeof(struct huffmanTree));
    for (int i = 0; i < numItems; i++) {
        nodes[i] = createHuffmanTreeNode(items[i].token, items[i].freq);
    }

    // Create huffman tree by combining smallest nodes
    while (numItems > 1) {
        qsort(nodes, numItems, sizeof(struct huffmanTree *), compareHuffmanTreeNodesByFrequency);
        // Create a new node with the two smallest frequency nodes as children
        struct huffmanTree *newNode = createHuffmanTreeNode(NULL, nodes[0]->freq + nodes[1]->freq);
        newNode->left = nodes[0];
        newNode->right = nodes[1];

        // Remove the two smallest nodes and add the new node
        nodes[0] = newNode;
        nodes[1] = nodes[numItems - 1];
        numItems--;
    }

    struct huffmanTree *huffmanRoot = nodes[0];

    // Free memory
    free(nodes);
    for (int i = 0; i < numItems; i++) {
        free(items[i].token);
    }
    free(items);
    CounterFree(c);

    return huffmanRoot;
}

// Task 4
// Encode the input file using the huffman tree
char *encode(struct huffmanTree *tree, char *inputFilename) {
    // Check if arguments are valid
    if (tree == NULL || inputFilename == NULL) {
        return NULL;
    }

    // Get text from input file
    struct file *inputFile = FileOpenToRead(inputFilename);
    char *inputText = FileToString(inputFile);
    FileClose(inputFile);
   
    // Get encoding for each unique letter in the input text
    struct huffmanEncodedData *encodedData = encodeData(tree, inputText);

    // Print the encoded data
    for (int i = 0; i < 9; i++) {
        printf("Letter: %c, Encoding: %s\n", encodedData[i].letter, encodedData[i].encoding);
    }

    // Create the encoded text from the encoded data
    int maxEncodedLength = strlen(inputText) * 8;
    char *encodedText = (char *)malloc(maxEncodedLength + 1);
    encodeText(encodedData, inputText, encodedText);

    // Free memory
    free(inputText);

    return encodedText;
}

// -------------------------------------------- Helper Functions --------------------------------------------
// Create a huffman tree node
struct huffmanTree *createHuffmanTreeNode(char *token, int frequency) {
    // Allocate memory for the node
    struct huffmanTree *newNode = (struct huffmanTree *)malloc(sizeof(struct huffmanTree));
    
    // Copy the token into the node
    if (token != NULL) {
        newNode->token = (char *)malloc(strlen(token) + 1);
        newNode->token = strcpy(newNode->token, token);
    } else {
        newNode->token = NULL;
    }

    // Set the frequency and children
    newNode->freq = frequency;
    newNode->left = newNode->right = NULL;

    return newNode;
}

// Compare two huffman tree nodes by frequency
int compareHuffmanTreeNodesByFrequency(const void* a, const void* b) {
    // Set data types
    struct huffmanTree* nodeA = *(struct huffmanTree**)a;
    struct huffmanTree* nodeB = *(struct huffmanTree**)b;
    
    // Compare frequencies and return accordingly
    if (nodeA->freq < nodeB->freq) {
        return -1;
    } else if (nodeA->freq > nodeB->freq) {
        return 1;
    } else {
        return 0;
    }
}

// Given the input text and huffman tree, create a data structure with the encodings of each unique letter
struct huffmanEncodedData* encodeData(struct huffmanTree* tree, char* inputText) {
    // Check if arguments are valid
    if (tree == NULL || inputText == NULL) {
        return NULL;
    }

    // Find the number of unique letters in the input text
    char uniqueLetters[27];
    findUniqueLetters(inputText, uniqueLetters);
    int textLength = strlen(uniqueLetters);

    // Allocate memory for the encoded data
    struct huffmanEncodedData *encodedData = (struct huffmanEncodedData *)malloc(textLength * sizeof(struct huffmanEncodedData));

    // /Add letters to encodedData
    for (int i = 0; i < textLength; i++) {
        char letter = uniqueLetters[i];
        struct huffmanEncodedData entry;
        entry.letter = letter;
        entry.encoding = NULL;
        encodedData[i] = entry;
    }

    // Add encodings to encodedData
    char *encoding = malloc(1);
    int index = 0;     
    populateHuffmanEncodeData(tree, encoding, encodedData, &index);

    return encodedData;
}

// Given the encoded data and input text, create the encoded text
char *encodeText(struct huffmanEncodedData *encodedData, char *inputText, char *outputText) {
    // Check if arguments are valid
    if (encodedData == NULL || inputText == NULL || outputText == NULL) {
        return NULL;
    }

    int inputLength = strlen(inputText);
    int outputIndex = 0;

    // Traverse the input text and replace each character with its Huffman encoding
    for (int i = 0; i < inputLength; i++) {
        char currentChar = inputText[i];

        // Find the Huffman encoding for the current character
        char *encoding = NULL;
        for (int j = 0; isalpha(encodedData[j].letter); j++) {
            if (encodedData[j].letter == currentChar) {
                encoding = encodedData[j].encoding;
                break;
            }
        }

        if (encoding != NULL) {
            // Append the encoding to the output text
            int encodingLength = strlen(encoding);
            for (int k = 0; k < encodingLength; k++) {
                outputText[outputIndex] = encoding[k];
                outputIndex++;
            }
        }
    }

    // Null-terminate the output text
    outputText[outputIndex] = '\0';

    return outputText;
}

// Traverse the huffman tree and populate the encoded data
void populateHuffmanEncodeData(struct huffmanTree *tree, char *encoding, struct huffmanEncodedData *encodedData, int *index) {
    // Traverse huffman tree recursively
    if(tree->left == NULL && tree->right == NULL) { // Leaf node found, enter data
        // Create entry struct
        struct huffmanEncodedData entry;
        // Get token of leaf node
        entry.letter = tree->token[0];
        // Copy encoding into entry
        entry.encoding = (char *)malloc(strlen(encoding) + 1);
        strcpy(entry.encoding, encoding);
        entry.encoding[strlen(encoding)] = '\0';
        encodedData[(*index)] = entry;
        // Increment index
        (*index)++;
    } else {
        // Recursive call for left subtree
        if (tree->left != NULL) {
            char *leftEncoding = (char *)malloc(strlen(encoding) + 2);
            strcpy(leftEncoding, encoding);
            strcat(leftEncoding, "0"); // Append '0' for left subtree
            populateHuffmanEncodeData(tree->left, leftEncoding, encodedData, index);
            free(leftEncoding); // Free the dynamically allocated array
        }
        // Recursive call for right subtree
        if (tree->right != NULL) {
            char *rightEncoding = (char *)malloc(strlen(encoding) + 2);
            strcpy(rightEncoding, encoding);
            strcat(rightEncoding, "1"); // Append '1' for right subtree
            populateHuffmanEncodeData(tree->right, rightEncoding, encodedData, index);
            free(rightEncoding); // Free the dynamically allocated array
        }
    }
}

// Given a file, return a string containing the file's contents
char *FileToString(File file) {
    // Check if argument is valid
    if (file == NULL) {
        return NULL;
    }

    char arr[MAX_TOKEN_LEN + 1];
    char *string = NULL;
    size_t stringSize = 0;

    while (FileReadToken(file, arr)) {
        size_t tokenLen = strlen(arr);

        // Calculate the new size, including the null-terminator
        size_t newSize = stringSize + tokenLen + 1;

        // Allocate a new buffer
        char *newString = (char *)realloc(string, newSize);
        if (newString == NULL) {
            free(string);
            return NULL;
        }

        // Copy the token into the new buffer
        strncpy(newString + stringSize, arr, tokenLen);

        // Update the string size and the null-terminator
        stringSize = newSize - 1;
        newString[stringSize] = '\0';

        string = newString;
    }

    return string;
}

// Given a string, find the unique letters in the string
void findUniqueLetters(const char *input, char *uniqueLetters) {
    // Initialize an array to keep track of encountered letters.
    bool encountered[26] = {false};
    int uniqueCount = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if ('a' <= input[i] && input[i] <= 'z') {
            int index = input[i] - 'a';
            if (!encountered[index]) {
                uniqueLetters[uniqueCount] = input[i];
                encountered[index] = true;
                uniqueCount++;
            }
        }
    }

    // Null-terminate the uniqueLetters array.
    uniqueLetters[uniqueCount] = '\0'; 
}

// Given a string, append a character to the end of the string
void appendString(char **output, char c, int *index) {
    if (*output != NULL) {
        size_t currentSize = strlen(*output);
        // Reallocate memory to accommodate the new character
        char *newOutput = (char *)realloc(*output, currentSize + 2); // +2 for the new character and null terminator
        if (newOutput == NULL) return;

        // Append and null terminate the string
        newOutput[currentSize] = c; 
        newOutput[currentSize + 1] = '\0';

        // Update the output pointer
        *output = newOutput; 
    }
}