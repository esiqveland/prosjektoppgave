#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "uthash/uthash.h"

#define DEFAULT_TERM_NUM 5
#define DEBUG 1

typedef struct postings_entry postings_entry;

typedef struct {
	char* word;
	uint32_t byte_offset;
	uint32_t occurences;
	uint32_t occurences_abstract;
    postings_entry* posting;
    UT_hash_handle hh;         /* makes this structure hashable */
} dictionary_entry;

/* query has a number of terms (strings) */
typedef struct {
    char* term;
    float score;
    UT_hash_handle hh;
} query;

typedef struct {
    int docid;
    float* scores; /* score for every term in the query for this doc */
    UT_hash_handle hh;
} doc_score;

struct postings_entry {
    uint32_t docId;
    uint32_t term_frequency;
    postings_entry* next;
};

dictionary_entry *dictionary = NULL;    /* important! initialize to NULL for uthash */

char* postings_file = "target/postings.txt";    /* filepath to postings.txt file */

void hash_dict_entry(dictionary_entry *dict_entry) {
    HASH_ADD_KEYPTR(hh, dictionary, dict_entry->word, strlen(dict_entry->word), dict_entry);
}

dictionary_entry* find_dict_entry(char* word) {
    dictionary_entry* dict_entry;
    HASH_FIND_STR(dictionary, word, dict_entry);
    if(dict_entry == NULL)
        printf("lookup: %s failed\n", word);
    return dict_entry;
}

postings_entry* init_alloc_postings_entry() {
    postings_entry* entry = malloc(sizeof(postings_entry));
    entry->next = NULL;
    return entry;
}
query* init_alloc_query() {
    query* query = malloc(sizeof(query));
    return query;
}
dictionary_entry* init_alloc_dictionary_entry() {
    dictionary_entry* dict = malloc(sizeof(dictionary_entry));
    dict->posting = NULL;
    dict->word = NULL;
    return dict;
}

// Reads the dictionary into memory
void build_dictionary(const char* input_file_dict) {
    FILE* f = fopen(input_file_dict,"r");
    char* buffer = malloc(sizeof(char)*64);
    while(!feof(f)) {
    	dictionary_entry* dict_entry = init_alloc_dictionary_entry();

    	fscanf(f, "%s %" SCNu32 " %" SCNu32 " %" SCNu32 "\n", buffer, &dict_entry->byte_offset,
               &dict_entry->occurences, &dict_entry->occurences_abstract);
        dict_entry->word = strdup(buffer);
        hash_dict_entry(dict_entry);
    }
    fclose(f);
}

void print_dictionary() {
    dictionary_entry *dict_entry, *tmp;
    
    for(dict_entry=dictionary; dict_entry != NULL; dict_entry=dict_entry->hh.next) {
//    HASH_ITER(hh, dictionary, dict_entry, tmp) {
        if(dict_entry->posting == NULL) {
            //continue;
        }
        printf("%s %" SCNu32 " %" SCNu32 " %" SCNu32 "\n", dict_entry->word, dict_entry->byte_offset,
                dict_entry->occurences, dict_entry->occurences_abstract);
        postings_entry* posting = dict_entry->posting;
        if (posting != NULL) {
            printf("\t");
        }
        while(posting != NULL) {
            printf("%d %d ",posting->docId, posting->term_frequency);
            posting = posting->next;
        }
        printf("\n");
    }
}

void add_term_to_query(query** query_dict, char* term) {
    // if query in hashtable, increase score:
    query* myq = NULL;

    if(DEBUG)
        printf("void add_term_to_query(query** query_dict, char* term, dictionary_entry* dict_entry): %s\n", term);

    if(*query_dict != NULL) {
        if(DEBUG)
            printf("query_dict not null\n");
        HASH_FIND_STR(*query_dict, term, myq);
    }
    if(myq == NULL) {
        myq = init_alloc_query();
        myq->term = strdup(term);
        myq->score = 1.0f;
        HASH_ADD_KEYPTR(hh, *query_dict, myq->term, strlen(myq->term), myq);
    } else {
        myq->score += 1.0f;
    }
}

void trim_string(char* querystr) {
    // Trim trailing space
    char* end = querystr + strlen(querystr) - 1;
    while(end > querystr && isspace(*end)) end--;
    
    // Write new null terminator
    *(end+1) = 0;
}
void split_query_into_terms(char* querystr, query** query_dict) {
    char* myquery = strdup(querystr);

    char* reentrant_saver;
    
    char* token;
    token = strtok_r(myquery, " \n\r\t", &reentrant_saver);
    
    while(token != NULL) {
        dictionary_entry* dict_entry = find_dict_entry(token);
        if(DEBUG)
            printf("found token: %s\n", token);

        if(dict_entry != NULL) {
            add_term_to_query(query_dict, token);
        }
        token = strtok_r(NULL, " \n\r\t", &reentrant_saver);
    }
    //free(myquery); // this crashes program?!
}

void print_query_struct(query** query_dict) {
    query* entry;
    for(entry=*query_dict; entry != NULL; entry=entry->hh.next) {
        printf("%s: %f\n", entry->term, entry->score);
    }
}

void build_postingslist(dictionary_entry* dict_entry) {
    // return if we have already read the postingslist for this term
    if(dict_entry->posting != NULL) {
        return;
    }
    if(dict_entry->occurences == 0) {
        dict_entry->posting = NULL;
        return;
    }
    
    FILE* f_postings = fopen(postings_file, "rb");
    fseek(f_postings, dict_entry->byte_offset, SEEK_SET); //SEEK_SET is beginning of file
    
    postings_entry* entry = init_alloc_postings_entry();
    dict_entry->posting = entry;
    postings_entry* prev;

    int i;
    for(i = 0; i < dict_entry->occurences; i++) {
        fread(entry, 8, 1, f_postings);
        entry->next = init_alloc_postings_entry();
        prev = entry;
        entry = entry->next;
    }
    //free(entry);
    prev->next = NULL;
    fclose(f_postings);
}

void handle_token(char* token) {
    if(DEBUG) {
        printf("handle token: %s\n", token);
    }
    dictionary_entry* dict_entry = find_dict_entry(token);
    build_postingslist(dict_entry);
}

void add_doc_score(doc_score* doc_scores, doc_score* doc_score_to_add) {
    HASH_ADD_INT(doc_scores, docid, doc_score_to_add);
}

doc_score* alloc_init_doc_score(int query_terms) {
    doc_score* score = malloc(sizeof(doc_score));
    score->scores = malloc(sizeof(float)*query_terms);
    int i;
    for(i = 0; i < query_terms; i++) {
        score->scores[i] = 0.0f;
    }
    return score;
}

doc_score* lookup_doc_score(doc_score* doc_score_hash, int docid) {
    doc_score* score;
    HASH_FIND_INT(doc_score_hash, &docid, score);
    return score;
}

void destroy_doc_score(doc_score* score) {
    if(score->scores != NULL)
        free(score->scores);
    score->scores = NULL;
}

void score_query(query* query_dict) {
    doc_score* doc_scores = NULL;

    query* entry;
    for(entry=query_dict; entry != NULL; entry=entry->hh.next) {

        dictionary_entry* dict_entry = find_dict_entry(entry->term);
        postings_entry* posting = dict_entry->posting;
 
        int j;
        for(j = 0; j < dict_entry->occurences; j++) {
            doc_score* score = lookup_doc_score(doc_scores, posting->docId);
            if(score == NULL) {
                score = alloc_init_doc_score(1);
                score->docid = posting->docId;
            }
        }

    }
}

void prefetch_tokens(query** query_dict) {
    query* entry;

    for(entry=*query_dict; entry != NULL; entry=entry->hh.next) {
        handle_token(entry->term);
    }

}

void search(char* querystr) {
    
    if(DEBUG)
        printf("given query: %s\n", querystr);
    query* query_dict = NULL;
    
    split_query_into_terms(querystr, &query_dict);
    //print_query_struct(&query_dict);

    //prefetch_tokens(&query_dict);
    //score_query(query_dict);
}

int main(int argc, char* argv[])
{
    build_dictionary("target/dictionary.txt");
    print_dictionary();
    search(argv[1]);
    print_dictionary();
}

