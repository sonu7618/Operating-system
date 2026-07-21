#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 10
#define SALT "c0urseW0rk_salt_2026"

typedef struct
{
    char username[32];
    unsigned long password_hash;
    int permission_level;
} UserRecord;

UserRecord user_db[MAX_USERS];
int user_count = 0;

// Generate an FNV-1a hash
unsigned long fnv1a_hash(const char *str)
{
    unsigned long hash = 2166136261UL;

    while (*str)
    {
        hash ^= (unsigned char)*str;
        hash *= 16777619UL;
        str++;
    }

    return hash;
}

// Hash the password with a salt
unsigned long hash_password(const char *password)
{
    char salted[128];

    snprintf(salted, sizeof(salted), "%s%s", SALT, password);

    return fnv1a_hash(salted);
}

// Register a new user
void register_user(const char *username, const char *password, int level)
{
    if (user_count >= MAX_USERS)
    {
        printf("User database is full.\n");
        return;
    }

    strncpy(user_db[user_count].username, username,
            sizeof(user_db[user_count].username) - 1);

    user_db[user_count].username[sizeof(user_db[user_count].username) - 1] = '\0';

    user_db[user_count].password_hash = hash_password(password);
    user_db[user_count].permission_level = level;

    user_count++;
}

// Authenticate a user
int authenticate(const char *username, const char *password)
{
    unsigned long attempt_hash = hash_password(password);

    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(user_db[i].username, username) == 0)
        {
            if (user_db[i].password_hash == attempt_hash)
            {
                return i;
            }

            return -1;
        }
    }

    return -2;
}

// Convert permission level to text
const char *level_name(int level)
{
    switch (level)
    {
        case 0:
            return "READ-ONLY";

        case 1:
            return "READ-WRITE";

        case 2:
            return "ADMIN";

        default:
            return "UNKNOWN";
    }
}

int main(void)
{
    // Register sample users
    register_user("alice", "P@ssw0rd123", 2);
    register_user("bob", "hunter2!", 1);
    register_user("carol", "letmein9", 0);

    printf("Registered Users\n\n");

    for (int i = 0; i < user_count; i++)
    {
        printf("Username : %s\n", user_db[i].username);
        printf("Hash     : %lu\n", user_db[i].password_hash);
        printf("Level    : %s\n\n",
               level_name(user_db[i].permission_level));
    }

    struct
    {
        const char *username;
        const char *password;
    } attempts[] =
    {
        {"alice", "P@ssw0rd123"},
        {"bob", "wrongpass"},
        {"dave", "whatever"},
        {"carol", "letmein9"}
    };

    printf("Authentication Results\n\n");

    for (int i = 0; i < 4; i++)
    {
        int result = authenticate(attempts[i].username,
                                  attempts[i].password);

        printf("User: %s -> ", attempts[i].username);

        if (result >= 0)
        {
            printf("SUCCESS (%s)\n",
                   level_name(user_db[result].permission_level));
        }
        else if (result == -1)
        {
            printf("FAILED (Incorrect Password)\n");
        }
        else
        {
            printf("FAILED (User Not Found)\n");
        }
    }

    return 0;
}