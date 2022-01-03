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
/**
 * Return 0 if succesfully returned all the persons
 * Returns -1 if an error has occured
 * Returns 1 if more persons are available
 * 
*/
int get_all_persons(person_t *persons, int chunk_number);

int db_init(void);
int db_close(void);
