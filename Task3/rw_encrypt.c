#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define AUDIT_LOG "audit.log"

// Record file operations in the audit log
void audit_log_event(const char *user, const char *action, const char *target)
{
    time_t now = time(NULL);
    char timebuf[64];

    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    FILE *log = fopen(AUDIT_LOG, "a");

    if (log == NULL)
    {
        perror("Unable to open audit log");
        return;
    }

    fprintf(log, "[%s] User=%s Action=%s Target=%s\n",
            timebuf, user, action, target);

    fclose(log);
}

// Encrypt or decrypt data using XOR
void xor_encrypt_decrypt(unsigned char *data, size_t len, const char *key)
{
    size_t key_len = strlen(key);

    for (size_t i = 0; i < len; i++)
    {
        data[i] ^= key[i % key_len];
    }
}

int main(void)
{
    const char *user = "sooo";
    const char *filename = "secure_document.txt";
    const char *encrypted_filename = "secure_document.enc";
    const char *decrypted_filename = "secure_document.dec";
    const char *key = "OS-C0urseW0rk-Key";

    FILE *fp;

    // Step 1: Append text to the file
    fp = fopen(filename, "a");

    if (fp == NULL)
    {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    fprintf(fp, "Appended line written using fopen()/fprintf() at run time.\n");

    fclose(fp);

    audit_log_event(user, "WRITE", filename);

    printf("Step 1: Data appended to '%s'\n", filename);

    // Step 2: Read the file
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    unsigned char *buffer = malloc(size + 1);

    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        fclose(fp);
        return EXIT_FAILURE;
    }

    size_t bytes_read = fread(buffer, 1, size, fp);

    buffer[bytes_read] = '\0';

    fclose(fp);

    audit_log_event(user, "READ", filename);

    printf("\nStep 2: File contents:\n%s\n", buffer);

    // Step 3: Encrypt the data
    unsigned char *enc_buffer = malloc(size);

    if (enc_buffer == NULL)
    {
        perror("Memory allocation failed");
        free(buffer);
        return EXIT_FAILURE;
    }

    memcpy(enc_buffer, buffer, size);

    xor_encrypt_decrypt(enc_buffer, size, key);

    fp = fopen(encrypted_filename, "wb");

    if (fp == NULL)
    {
        perror("Unable to create encrypted file");
        free(buffer);
        free(enc_buffer);
        return EXIT_FAILURE;
    }

    fwrite(enc_buffer, 1, size, fp);

    fclose(fp);

    audit_log_event(user, "ENCRYPT", encrypted_filename);

    printf("Step 3: Encrypted file saved as '%s'\n", encrypted_filename);

    // Step 4: Decrypt the data
    unsigned char *dec_buffer = malloc(size);

    if (dec_buffer == NULL)
    {
        perror("Memory allocation failed");
        free(buffer);
        free(enc_buffer);
        return EXIT_FAILURE;
    }

    memcpy(dec_buffer, enc_buffer, size);

    xor_encrypt_decrypt(dec_buffer, size, key);

    fp = fopen(decrypted_filename, "wb");

    if (fp == NULL)
    {
        perror("Unable to create decrypted file");
        free(buffer);
        free(enc_buffer);
        free(dec_buffer);
        return EXIT_FAILURE;
    }

    fwrite(dec_buffer, 1, size, fp);

    fclose(fp);

    audit_log_event(user, "DECRYPT", decrypted_filename);

    printf("Step 4: Decrypted file saved as '%s'\n", decrypted_filename);

    // Step 5: Verify the decrypted data
    int match = (memcmp(buffer, dec_buffer, size) == 0);

    printf("Step 5: Verification = %s\n",
           match ? "SUCCESS" : "FAILED");

    // Step 6: Create and delete a temporary file
    FILE *temp = fopen("temp_scratch.txt", "w");

    if (temp != NULL)
    {
        fprintf(temp, "Temporary file for deletion demonstration.\n");

        fclose(temp);

        audit_log_event(user, "CREATE", "temp_scratch.txt");

        if (remove("temp_scratch.txt") == 0)
        {
            audit_log_event(user, "DELETE", "temp_scratch.txt");

            printf("Step 6: Temporary file deleted successfully.\n");
        }
    }

    // Display the audit log
    printf("\nAudit Log:\n");

    fp = fopen(AUDIT_LOG, "r");

    if (fp != NULL)
    {
        char line[256];

        while (fgets(line, sizeof(line), fp))
        {
            printf("%s", line);
        }

        fclose(fp);
    }

    free(buffer);
    free(enc_buffer);
    free(dec_buffer);

    return 0;
}