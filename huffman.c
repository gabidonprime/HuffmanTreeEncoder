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
char *FileToString(File file);
char *encodeCharacter(struct huffmanTree *tree, char inputCharacter);
bool encodeCharacterDFS(struct huffmanTree *node, char inputCharacter, char *path, char **encoding);

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
   
    // Initialize a buffer for encoded text
    char *encodedText = malloc(1);
    encodedText[0] = '\0';

    // Encode the text based on the Huffman tree
    for (int i = 0; i < strlen(inputText); i++) {
        // Get encoding of character
        char *encoding = encodeCharacter(tree, inputText[i]);

        // Allocate enough memory for the result, including previous contents
        char *newEncodedText = (char *)malloc(strlen(encodedText) + strlen(encoding) + 1);
        strcpy(newEncodedText, encodedText);

        // Append the encoding to the newEncodedText
        strcat(newEncodedText, encoding);

        // Free the old encodedText and update the pointer
        free(encodedText);
        encodedText = newEncodedText;

        // Free the memory allocated for encoding
        free(encoding);
    }

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

// Encode a character using the huffman tree
char *encodeCharacter(struct huffmanTree *tree, char inputCharacter) {
    // Check if arguments are valid
    if (tree == NULL) {
        return NULL;
    }

    // Allocate memory for the encoding
    char *encoding = malloc(1);
    encoding[0] = '\0';
    char *path = "";

    // Traverse the tree to find the character
    encodeCharacterDFS(tree, inputCharacter, path, &encoding);

    return encoding;
}

// DFS to find the encoding of a character
bool encodeCharacterDFS(struct huffmanTree *node, char inputCharacter, char *path, char **encoding) {
    // If tree is invalid
    if (node == NULL) {
        return false;
    }

    // If the node contains the character, store the encoding and return true
    if (node->token && node->token[0] == inputCharacter) {
        *encoding = strdup(path);
        return true;
    }

    // Maximum path length of 10
    char leftPath[10];
    char rightPath[10];

    // Recursively search the left and right subtrees
    if (node->left) {
        strcpy(leftPath, path);
        strcat(leftPath, "0");
        if (encodeCharacterDFS(node->left, inputCharacter, leftPath, encoding)) {
            return true;
        }
    }

    if (node->right) {
        strcpy(rightPath, path);
        strcat(rightPath, "1");
        if (encodeCharacterDFS(node->right, inputCharacter, rightPath, encoding)) {
            return true;
        }
    }

    return false; // Character not found in this subtree
}