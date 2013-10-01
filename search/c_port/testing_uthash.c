#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uthash/uthash.h"

#define DEFAULT_TERM_NUM 5
#define DEBUG 1

/* query has a number of terms (strings) */
typedef struct {
    char* term;
    float score;
    UT_hash_handle hhq;
} query;

query* init_alloc_query() {
    query* query = malloc(sizeof(query));
    query->term = NULL;
    return query;
}

void hash_query_entry(query** query_dict, query* myq) {
    printf("hash q entry: key:%s leng:%lu\n", myq->term, strlen(myq->term));
    
    HASH_ADD_KEYPTR(hhq, *query_dict, myq->term, strlen(myq->term), myq);
    printf("hash q entry after addition: key:%s leng:%lu\n", myq->term, strlen(myq->term));    
}

void proxy_query_entry(query** query_dict, query* myq) {
    hash_query_entry(query_dict, myq);
}

void add_term_to_query(query** query_dict, char* term) {
    // if query in hashtable, increase score:
    query* myq = NULL;

    if(DEBUG)
        printf("void add_term_to_query(query** query_dict, char* term, dictionary_entry* dict_entry): %s\n", term);

    if(*query_dict != NULL) {
        if(DEBUG)
            printf("query_dict not null, term: %s\n", term);
//        HASH_FIND_STR(query_dict, term, myq);
        HASH_FIND(hhq, *query_dict, term, strlen(term), myq);
    }
    if(myq == NULL) {
        myq = malloc(sizeof(query));
        myq->term = strdup(term);
        myq->score = 1.0f;
        hash_query_entry(query_dict, myq);
    } else {
        myq->score += 1.0f;
    }
}
void split_query_into_terms(query** query_dict, char* querystr) {
    char* myquery = strdup(querystr);

    query* adding = malloc(sizeof(query));
    char* keystr = "testing";
    adding->term = strdup(querystr);
    adding->score = 1.0f;
    //proxy_query_entry(query_dict, adding);
    //add_term_to_query(query_dict, strdup(querystr));
    //return;
    char* reentrant_saver;
    
    char* token;
    token = strtok_r(myquery, " \n\r\t", &reentrant_saver);
    
    while(token != NULL) {
        if(DEBUG)
            printf("found token in dictionary: %s\n", token);
        add_term_to_query(query_dict, token);
        token = strtok_r(NULL, " \n\r\t", &reentrant_saver);
    }
    //free(myquery); // this crashes program?!
}

void print_query_struct(query** query_dict) {
    query* entry;
    printf("query_dict:\n");
    for(entry=*query_dict; entry != NULL; entry=entry->hhq.next) {
        printf("\t%s: %f\n", entry->term, entry->score);
    }
}

void search(char* querystr) {
    
    if(DEBUG)
        printf("given query: %s\n", querystr);

    query *query_dict = NULL;

    split_query_into_terms(&query_dict, querystr);
    print_query_struct(&query_dict);
    
    //split_query_into_terms(&query_dict, querystr);
    //print_query_struct(&query_dict);

    //prefetch_tokens(&query_dict);
    //score_query(query_dict);
}


int main(int argc, char* argv[])
{
    //build_dictionary("target/dictionary.txt");
    //print_dictionary();
    search(argv[1]);
    
    //print_dictionary();
}
