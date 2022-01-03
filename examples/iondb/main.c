#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "shell.h"
#include "store.h"
#define CHUNK_SIZE 5

void printPerson(person_t p)
{
    printf("Person{id=%.14s,lat=%f,lon=%f,status=%d,timestamp=%" PRIu64 "}\n", p.id, p.lat, p.lat, p.status, p.timestamp);
}

static int _init(int argc, char **argv)
{

    (void)argc;
    (void)argv;
    return db_init();
}
static int _close(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return db_close();
}

static int _hello(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("%s\n", "Hello world");
    return 1;
}

static int _add(int argc, char **argv)
{
    if (argc < 2)
    {
        puts("Usage: add [id]");
        return 1;
    }
    person_t p = {.lat = 3, .lon = 4, .status = 1, .timestamp = 99};
    memcpy(p.id, argv[1], 14);
    puts("Saving person:");
    printPerson(p);
    int ret = save_person(p);
    return ret;
}
static int _findAll(int argc, char **argv)
{
    printf("Find all persons\n");
    person_t persons[CHUNK_SIZE] = {0};
    int i;
    for (i = 0; i < CHUNK_SIZE; i++)
    {
        printPerson(persons[i]);
    }
    int ret = get_all_persons(persons, 0);
    return ret;
}

static int _find(int argc, char **argv)
{
    if (argc < 2)
    {
        puts("Usage: find [id]");
        return 1;
    }
    printf("Querying person with id=%s\n", argv[1]);
    person_t p = {0};
    int ret = find_person_by_id(argv[1], &p);
    if (ret != 0)
    {
        printf("No person found with id=%s\n", argv[1]);
    }
    else
    {
        printPerson(p);
    }
    return ret;
}

static int _print(int argc, char **argv)
{
    person_t p = {.id = {'I', 'd', '\0'}, .lat = 3, .lon = 4, .status = 1, .timestamp = 99};
    puts("Saving person:");
    printPerson(p);
    int ret = save_person(p);
    return ret;
}
static const shell_command_t shell_commands[] = {
    {"init", "start the db", _init},
    {"close", "close the db", _close},
    {"add", "add person with given id", _add},
    {"find", "find a person by id", _find},
    {"findAll", "find all persons", _findAll},
    {"print", "print all persons", _print},
    {"hello", "Prints hello world", _hello},
    {NULL, NULL, NULL}};

int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
