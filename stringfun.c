#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int count_words(char *, int, int);
//add additional prototypes here
int reverse_string(char *, int, int);
int print_words(char *, int, int);
int replace_string(char *, int, int, char *, char *);

int setup_buff(char *buff, char *user_str, int len) {
    int count = 0;
    char prev = ' ';
    if (!buff || !user_str) return -2;

    while (*user_str && count < len) {
        if (*user_str != ' ' && *user_str != '\t') {
            if (prev == ' ') {
                if (count > 0) buff[count++] = ' ';
            }
            buff[count++] = *user_str;
            prev = *user_str;
        } else {
            prev = ' ';
        }
        user_str++;
    }

    if (*user_str) return -1;

    while (count < len) {
        buff[count++] = '.';
    }

    return count;
}

void print_buff(char *buff, int len) {
    printf("Buffer:  ");
    for (int i = 0; i < len; i++) {
        printf("%c", *(buff + i));
    }
    printf("\n");
}

void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int len, int str_len) {
    if (str_len > len || str_len == 0) return -1;

    int word_count = 0;
    int in_word = 0;

    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) != ' ') {
            if (!in_word) {
                in_word = 1;
                word_count++;
            }
        } else {
            in_word = 0;
        }
    }

    return word_count;
}

int reverse_string(char *buff, int len, int str_len) {
    if (str_len > len || str_len == 0) return -1;

    for (int i = 0; i < str_len / 2; i++) {
        char temp = *(buff + i);
        *(buff + i) = *(buff + str_len - i - 1);
        *(buff + str_len - i - 1) = temp;
    }

    return 0;
}

int print_words(char *buff, int len, int str_len) {
    if (str_len > len || str_len == 0) return -1;

    int word_count = 0;
    int char_count = 0;
    int in_word = 0;

    printf("Word Print\n----------\n");

    for (int i = 0; i < str_len; i++) {
        char c = *(buff + i);

        if (c != ' ') {
            if (!in_word) {
                in_word = 1;
                word_count++;
                if (word_count > 1) printf("\n");
                printf("%d. ", word_count);
            }
            printf("%c", c);
            char_count++;
        } else {
            if (in_word) {
                printf("(%d)", char_count);
                char_count = 0;
                in_word = 0;
            }
        }
    }

    if (in_word) {
        printf("(%d)", char_count);
    }

    printf("\n");
    return word_count;
}

int replace_string(char *buff, int len, int str_len, char *old, char *new) {
    int old_len = 0, new_len = 0;
    while (*(old + old_len)) old_len++;
    while (*(new + new_len)) new_len++;

    char *found = NULL;
    for (int i = 0; i <= str_len - old_len; i++) {
        int match = 1;
        for (int j = 0; j < old_len; j++) {
            if (*(buff + i + j) != *(old + j)) {
                match = 0;
                break;
            }
        }
        if (match) {
            found = buff + i;
            break;
        }
    }

    if (!found) return -1;

    if (str_len - old_len + new_len > len) return -1;

    for (int i = str_len - 1; i >= (found - buff) + old_len; i--) {
        *(buff + i + new_len - old_len) = *(buff + i);
    }

    for (int i = 0; i < new_len; i++) {
        *(found + i) = *(new + i);
    }

    return 0;
}

int main(int argc, char *argv[]) {

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int rc;                //used for return codes
    int user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //This is safe because argc ensures there are enough arguments before accessing argv[1]
    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1] + 1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h') {
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      The if statement ensures the user provides a string as the second argument (argv[2])
    //       Without this check the program would access argv[2] even if it doesn't exist
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char *)malloc(BUFFER_SZ);
    if (!buff) exit(99);

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0) {
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt) {
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0) {
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            rc = reverse_string(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error reversing string, rc = %d", rc);
                exit(2);
            }
            break;
        case 'w':
            rc = print_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error printing words, rc = %d", rc);
                exit(2);
            }
            break;
        case 'x':
            if (argc < 5) {
                usage(argv[0]);
                exit(1);
            }
            rc = replace_string(buff, BUFFER_SZ, user_str_len, argv[3], argv[4]);
            if (rc < 0) {
                printf("Not Implemented!\n");
                exit(3);
            }
            break;
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff, BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          Providing both the length and pointer is good practice because
//          it allows the same functions to work with buffers of different sizes
//          and ensures the functions dont read or write beyond the allocated memory
//

