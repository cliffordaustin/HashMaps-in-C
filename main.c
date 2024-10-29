#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FNV_OFFSET_BASIS 2166136261u // FNV-1 Constant value used in our function
#define FNV_PRIME 16777619u // FNV-1 Constant value used in our function
#define TABLE_SIZE 10000

// generic entry, so our data is a void pointer
typedef struct entry {
    void *data;
    char *key;
    struct entry *next;
    // since we are using separate chaining, we need to point to the next entry if there is a collision
} entry;

typedef struct phonebook {
    char *name;
    char *number;
} phonebook;

u_int32_t hash_func(const char *key) {
    const u_int32_t len = strlen(key);
    u_int32_t hash = FNV_OFFSET_BASIS;

    for (u_int32_t i = 0; i < len; i++) {
        hash ^= (u_int8_t) key[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

entry *table[TABLE_SIZE] = {};

void init(void) {
    // Initializing our table with NULL entries
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = NULL;
    }
}

void print_table(void (print_data_func)(void *)) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        const entry *entry = table[i];
        if (entry == NULL) {
            continue;
        }

        printf("%d: ", i);
        while (entry != NULL) {
            print_data_func(entry->data);
            entry = entry->next;
        }

        printf("\n");
    }
}

entry *create_entry(void *data, u_int32_t (hash_func)(const char *), char *key) {
    const int index = hash_func(key) % TABLE_SIZE;
    entry *new = malloc(sizeof(*new));
    new->data = data;
    new->key = key;
    new->next = NULL;

    if (table[index] != NULL) {
        new->next = table[index];
    }

    table[index] = new;

    return new;
}

entry *get_entry(const char *key, u_int32_t (hash_func)(const char *)) {
    const int index = hash_func(key) % TABLE_SIZE;
    entry *temp = table[index];

    while (temp != NULL && strcmp(temp->key, key) != 0) {
        temp = temp->next;
    }

    if (temp == NULL) {
        fprintf(stderr, "%s not found in the table\n", key);
    }

    return temp;
}

void delete_entry(const char *key, u_int32_t (hash_func)(const char *)) {
    const int index = hash_func(key) % TABLE_SIZE;
    entry *temp = table[index];
    entry *prev = NULL;

    while (temp != NULL && strcmp(temp->key, key) != 0) {
        prev = temp;
        temp = temp->next;
    }

    if (prev == NULL) {
        table[index] = temp->next;
    } else if (temp != NULL) {
        prev->next = temp->next;
        table[index] = prev;
    } else {
        fprintf(stderr, "%s not found in the table\n", key);
    }
}

void print_phonebook_data(void *data) {
    const phonebook *phone = (phonebook *) data;
    printf("|%s, %s| -> ", phone->name, phone->number);
}

float collision_rate(void) {
    float collions = 0;
    float no_collions = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (table[i] != NULL && table[i]->next != NULL) {
            collions++;
        } else if (table[i] != NULL && table[i]->next == NULL) {
            no_collions++;
        }
    }

    return collions / (collions + no_collions) * 100;
}

// Helper function for trimming whitespace
char *trimwhitespace(char *str) {
    // Trim leading space
    while (isspace((unsigned char) *str)) str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

int main(const int argc, char **argv) {
    init();

    if (argc != 2) {
        fprintf(stderr, "No filename");
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Failed to open file");
        return EXIT_FAILURE;
    }

    // this is to hold the user name, which will also be our key
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // our phonebook data looks like so: <name> - <number>
        // strchr get the string starting from the character specified, so in our case, that is the '-'
        // which is something like so: - <number>
        char *dash_position = strchr(buffer, '-');

        if (dash_position != NULL) {
            // we remove the '-' by just setting the first, item which '-' to '\0'(null character).
            *dash_position = '\0';

            // Then we get the phonenumber simply by just going one step forward the string literal.
            char *phone_number = dash_position + 1;
            // To make sure we are removing the whitespace that comes before the actual number, we just do a while loop until we get to a no whitespace character
            while (*phone_number == ' ') {
                phone_number++;
            }

            //We then trim any whitespace using that comes after the name of the phonenumber using our `trimwhitespace` helper function.
            trimwhitespace(buffer);
            trimwhitespace(phone_number);

            // we allocate memory for our phone
            phonebook *phone = malloc(sizeof(*phone));
            // we are using `strdup` to creat a copy of our name and number string.
            // this is to make sure we are not referencing a pointer to our buffer, which changes for each loop.
            phone->name = strdup(buffer);
            phone->number = strdup(phone_number);

            // then we create an entry into our table using our `create_entry` function from earlier.
            create_entry(phone, hash_func, phone->name);
        } else {
            // basically invalid entry.
            printf("Dash not found in the input string.\n");
        }
    }

    print_table(print_phonebook_data);

    const float rate = collision_rate();

    printf("########################\n");

    printf("Collision Rate = %.2f%%\n", rate);

    fclose(file);

    return 0;
}
