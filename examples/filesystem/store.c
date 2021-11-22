#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#define root "/sda"

int hello(void)
{
    printf("%s\n", "Hello world");
    return 0;
}
// int save(char *key, int value) { return 0; }
int save(char *key, char *value)
{
    char *path = strcat(root, key);
#if defined(MODULE_NEWLIB) || defined(MODULE_PICOLIBC)
    FILE *f = fopen(path, "w+");
    if (f == NULL)
    {
        printf("error while trying to create %s\n", path);
        return 1;
    }
    if (fwrite(path, 1, strlen(value, f)) != value)
    {
        puts("Error while writing");
    }
    fclose(f);
#else
    int fd = open(path, O_RDWR | O_CREAT);
    if (fd < 0)
    {
        printf("error while trying to create file %s\n", path);
        return 1;
    }
    if (write(fd, path, strlen(value)) != (ssize_t)strlen(value))
    {
        puts("Error while writing");
    }
    close(fd);
#endif
    return 0;
}

// int load(char *key, int value) { return 0; }
int load(char *key, char *value)
{
    return 0;
}