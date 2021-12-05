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

int db_init(void);

int save_person(person_t person);

// int get_all_persons(person_t* persons);

person_t find_person_by_id(char *id);