#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FNV_OFFSET_BASIS 2166136261u
#define FNV_PRIME 16777619u
#define TABLE_SIZE 100000

typedef struct entry {
    void *data;
    char* key;
    struct entry *next;
} entry;

typedef struct phonebook {
    char* name;
    char* number;
} phonebook;

u_int32_t hash_func(const char *key) {
    const u_int32_t len = strlen(key);
    u_int32_t hash = FNV_OFFSET_BASIS;

    for (u_int32_t i = 0; i < len; i++)
    {
        hash ^= (u_int8_t)key[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

entry *table[TABLE_SIZE] = {};

void init(void) {
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

entry *create_entry(void *data, u_int32_t (hash_func)(const char*), char* key) {
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
    const phonebook *phone = (phonebook*)data;
    printf("|%s, %s| -> ", phone->name, phone->number);
}

char *trimwhitespace(char *str)
{
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    char *end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
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

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char *dash_position = strchr(buffer, '-');
        if (dash_position != NULL) {
            *dash_position = '\0';

            char *phone_number = dash_position + 1;
            while (*phone_number == ' ') {
                phone_number++;
            }

            trimwhitespace(buffer);
            trimwhitespace(phone_number);

            phonebook *phone = malloc(sizeof(*phone));
            phone->name = strdup(buffer);
            phone->number = strdup(phone_number);

            create_entry(phone, hash_func, phone->name);
        } else {
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
