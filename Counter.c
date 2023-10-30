// Implementation of the Counter ADT
// COMPLETE (memory leakage occuring)

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Counter.h"
#include "huffman.h"

// Structs definiton
struct counter {
	struct huffmanTree *root;
};

// Helper functions
struct huffmanTree *huffmanTreeNew(char *token);
struct huffmanTree *inserthuffmanTree(struct huffmanTree *root, char *token);
void freeHuffmanTree(struct huffmanTree *root);
int countDistinctTokens(struct huffmanTree *root);
int findTokenFrequency(struct huffmanTree *root, const char *token);
void collectItems(struct huffmanTree *root, struct item **items, int *index);

// Returns a new empty counter with no tokens
Counter CounterNew(void) {
    Counter newCounter = (Counter)malloc(sizeof(struct counter));
    newCounter->root = NULL;
    return newCounter;
}

// Frees all memory allocated to the given counter
void CounterFree(Counter c) {
    if (c == NULL) {
        return;
    }

    freeHuffmanTree(c->root);
    c->root = NULL;
    free(c);
}

// Adds an occurrence of the given token to the counter
void CounterAdd(Counter c, char *token) {
	// Check arguments are valid
    if (c == NULL || token == NULL) {
        printf("Invalid arguments\n");
		return;
    }

    // If tree is empty, insert huffman tree node at root
    if (c->root == NULL) {
        c->root = huffmanTreeNew(token);
        return;
    }

    // Otherwise, insert token into tree
	inserthuffmanTree(c->root, token);
}

// Returns the number of distinct tokens added to the counter
int CounterNumItems(Counter c) {
	// Check arguments are valid
    if (c == NULL || c->root == NULL) {
        return 0;
    }
    
	return countDistinctTokens(c->root);
} 

// Returns the frequency of the given token
int CounterGet(Counter c, char *token) {
    if (c == NULL || c->root == NULL || token == NULL) {
        return 0;
    }

    return findTokenFrequency(c->root, token);
}

// Returns a dynamically allocated array containing a copy of each distinct token in the counter and its count (in any order), and sets *numItems to the number of distinct tokens.
struct item *CounterItems(Counter c, int *numItems) {
    // Check arguments are valid
    if (c == NULL || c->root == NULL) {
        *numItems = 0;
        return NULL;
    }

    // Collect items
    *numItems = CounterNumItems(c);
    struct item *items = (struct item *)malloc(sizeof(struct item) * (*numItems));
    int index = 0;
    collectItems(c->root, &items, &index);

    // Duplicate items
    struct item *resultItems = (struct item *)malloc(sizeof(struct item) * (*numItems));
    for (int i = 0; i < *numItems; i++) {
        resultItems[i].token = strdup(items[i].token);
        resultItems[i].freq = items[i].freq;
    }

    // Free the original items array and return the duplicated items
    free(items);
    return resultItems;
}

// -------------------------------------------- Helper Functions --------------------------------------------

// Creates a new huffman tree node
struct huffmanTree *huffmanTreeNew(char *token) {
    // Allocate memory for new node
    struct huffmanTree *newNode = (struct huffmanTree *)malloc(sizeof(struct huffmanTree));
    if (newNode) {
        newNode->token = (char *)malloc(strlen(token) + 1);
        if (newNode->token) {
            strncpy(newNode->token, token, strlen(token) + 1);
            newNode->freq = 1;
            newNode->left = newNode->right = NULL;
        } else {
            free(newNode);
            newNode = NULL;
        }
    }
    return newNode;
}

// Inserts a huffman tree node into the tree
struct huffmanTree *inserthuffmanTree(struct huffmanTree *root, char *token) {
    // If tree is empty, insert huffman tree node at root
    if (root == NULL) {
        return huffmanTreeNew(token);
    }
    // Compare token to root token
    int cmp = strcmp(token, root->token);
    if (cmp == 0) { // Token already exists, increment frequency
        root->freq++;
    } else if (cmp < 0) { // Token is smaller, go to left subtree
        root->left = inserthuffmanTree(root->left, token);
    } else { // Token is larger, go to right subtree
        root->right = inserthuffmanTree(root->right, token);
    }
    return root;
}

// Free huffman tree data structure
void freeHuffmanTree(struct huffmanTree *root) {
    if (root == NULL) {
        return;
    }

    freeHuffmanTree(root->left);
    freeHuffmanTree(root->right);

    free(root->token);
    free(root);
}

// Counts the number of distinct tokens in the counter
int countDistinctTokens(struct huffmanTree *root) {
	// Base case
    if (root == NULL) {
		return 0;
	}

    // Recursively count distinct tokens in left and right subtrees
	int leftCount = countDistinctTokens(root->left);
	int rightCount = countDistinctTokens(root->right);

	return leftCount + rightCount + 1;
}

// Finds the frequency of a given token in the counter
int findTokenFrequency(struct huffmanTree *root, const char *token) {
    // Base case
    if (root == NULL) {
        return 0;
    }
    // Compare token to root token
    int cmp = strcmp(token, root->token);
    if (cmp == 0) {
        return root->freq;
    } else if (cmp < 0) {
        return findTokenFrequency(root->left, token);
    } else {
        return findTokenFrequency(root->right, token);
    }
}

// Collects all items in the counter
void collectItems(struct huffmanTree *root, struct item **items, int *index) {
    // Base case
    if (root == NULL) {
        return;
    }
	// Traverse left subtree
    collectItems(root->left, items, index);

    (*items)[*index].token = root->token;
    (*items)[*index].freq = root->freq;
    (*index)++;

    // Traverse right subtree
    collectItems(root->right, items, index);
}