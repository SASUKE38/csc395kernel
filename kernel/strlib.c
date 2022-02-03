// Retuns the length of a string. (strlen)
int stringlen(const char* str) {
  int result = 0;
  while (str[result] != '\0') {
    result++;
  }
  return result;
}