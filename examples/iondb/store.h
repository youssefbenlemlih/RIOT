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
#endif

int save_person(person_t person);
int db_init(void);
int db_close(void);