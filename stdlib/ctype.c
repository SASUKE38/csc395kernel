// Returns 1 if c is a letter, 0 otherwise.
int isalpha(int c) {
  if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122)) return 1;
  else return 0;
}

// Returns 1 if c is a digit, 0 otherwise.
int isdigit(int c) {
  if (c >= 48 && c <= 57) return 1;
  else return 0;
}

// Returns 1 if c is a lowercase letter, 0 otherwise.
int islower(int c) {
  if (c >= 65 && c <= 90) return 1;
  else return 0;
}

// Returns 1 if c is an uppercae letter, 0 otherwise.
int isupper(int c) {
  if (c >= 97 && c <= 122) return 1;
  else return 0;
}

// Returns the corresponding uppercase letter if c is a lowercase letter, c otherwise.
int toupper(int c) {
  if (isupper(c) == 1) {
    return c - 32;
  }
  else return c;
}

// Returns the corresponding lowercase letter if c is a uppercase letter, c otherwise.
int tolower(int c) {
  if (islower(c) == 1) {
    return c + 32;
  }
  else return c;
}

// Returns 1 if c is a whitespace, 0 otherwise.
int isspace(int c) {
  if ((c >= 9 && c <= 13) || c == 32) return 1;
  else return 0;
}
