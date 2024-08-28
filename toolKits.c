#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

int isValidCommand(const char *str) {
    const char *patterns[] = {
        "^ALIVE\\s*$",//0
        "^TIME\\s*$",//1
        "^USERS\\s*$",//2
        "^FILE (\"[^\"]+\"|[^\" ]+)(\\s*)$",//3
        "^SUM\\s*\\(\\s*([+-]?([0-9]*[.])?[0-9]+)\\s*,\\s*([+-]?([0-9]*[.])?[0-9]+)\\s*\\)\\s*$",//4
        "^DONE\\s*$",//5
        "^TERMINATE\\s*$",//6
    };
    
    int result;
    regex_t regex;
    
    for (int i = 0; i < 7; i++) {
        regcomp(&regex, patterns[i], REG_EXTENDED | REG_NOSUB);
        result = regexec(&regex, str, 0, NULL, 0);
        regfree(&regex); 
        if (result == 0) {
            return i;
        }
    }
    return -1;
}

void removeSpaces(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src != ' ') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}
void removeTrailingSpaces(char *str) {
    int len = strlen(str);
    while (len > 0 && str[len - 1] == ' ') {
        str[--len] = '\0';
    }
}

void formatNumber(char *time_str) {
    char* end_ptr = time_str + strlen(time_str) - 1;
    while (*end_ptr == '0') {
        *end_ptr-- = '\0';
    }
    if (*end_ptr == '.') {
        *end_ptr = '\0';
    }
}