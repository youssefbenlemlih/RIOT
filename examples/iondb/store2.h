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

int save_person2(person_t person);

// int get_all_persons(person_t* persons);
