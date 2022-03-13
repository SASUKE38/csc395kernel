#pragma once

// Returns 1 if c is a letter, 0 otherwise.
int isalpha(int c);

// Returns 1 if c is a digit, 0 otherwise.
int isdigit(int c);

// Returns 1 if c is a lowercase letter, 0 otherwise.
int islower(int c);

// Returns 1 if c is an uppercae letter, 0 otherwise.
int isupper(int c);

// Returns the corresponding uppercase letter if c is a lowercase letter, c otherwise.
int toupper(int c);

// Returns the corresponding lowercase letter if c is a uppercase letter, c otherwise.
int tolower(int c);

// Returns 1 if c is a whitespace, 0 otherwise.
int isspace(int c);
