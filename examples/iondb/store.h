#ifndef person_t
struct person
{
    char id[14];
    int status;
    double lat;
    double lon;
    uint64_t timestamp;
};
typedef struct person person_t;
typedef struct person *personPtr;
#endif

int save_person(person_t person);
int find_person_by_id(char *id, personPtr out);
int db_init(void);
int db_close(void);
