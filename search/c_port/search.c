#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "uthash/uthash.h"

#define DEFAULT_TERM_NUM 5

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
    unsigned int current_size;
    unsigned int max_size;
    char** terms;
    int* scores;
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
    //HASH_ADD_STR(dictionary, word, dict_entry);
    HASH_ADD_KEYPTR(hh, dictionary, dict_entry->word, strlen(dict_entry->word), dict_entry);
}

dictionary_entry* find_dict_entry(const char* word) {
    dictionary_entry* dict_entry;
    HASH_FIND_STR(dictionary, word, dict_entry);
    if(dict_entry == NULL)
        fprintf(stdout, "lookup: %s failed\n", word);
    return dict_entry;
}

postings_entry* init_alloc_postings_entry() {
    postings_entry* entry = malloc(sizeof(postings_entry));
    entry->next = NULL;
    return entry;
}
query* init_alloc_query() {
    query* query = malloc(sizeof(query));
    query->terms = malloc(sizeof(char*)*DEFAULT_TERM_NUM);
    query->current_size = 0;
    query->max_size = DEFAULT_TERM_NUM;
    return query;
}
dictionary_entry* init_alloc_dictionary_entry() {
    dictionary_entry* dict = malloc(sizeof(dictionary_entry));
    dict->posting = NULL;
    dict->word = malloc(sizeof(char)*64);
    return dict;
}

// Reads the dictionary into memory
void build_dictionary(const char* input_file_dict) {
    FILE* f = fopen(input_file_dict,"r");
    
    while(!feof(f)) {
    	dictionary_entry* dict_entry = init_alloc_dictionary_entry();

    	fscanf(f, "%s %" SCNu32 " %" SCNu32 " %" SCNu32 "\n", dict_entry->word, &dict_entry->byte_offset,
               &dict_entry->occurences, &dict_entry->occurences_abstract);
        
        hash_dict_entry(dict_entry);
    }
    fclose(f);
}

void print_dictionary() {
    dictionary_entry* dict_entry;
    
    for(dict_entry=dictionary; dict_entry != NULL; dict_entry=dict_entry->hh.next) {
        if(dict_entry->posting == NULL) {
            continue;
        }
        fprintf(stdout, "%s %" SCNu32 " %" SCNu32 " %" SCNu32 "\n", dict_entry->word, dict_entry->byte_offset,
                dict_entry->occurences, dict_entry->occurences_abstract);
        postings_entry* posting = dict_entry->posting;
        if (posting != NULL) {
            fprintf(stdout, "\t");
        }
        while(posting != NULL) {
            fprintf(stdout, "%d %d ",posting->docId, posting->term_frequency);
            posting = posting->next;
        }
        fprintf(stdout, "\n");
    }
}

void print_query_struct(query* query) {
    int i;
    for(i = 0; i < query->current_size; i++) {
        fprintf(stdout, "%d: %s\n", i, query->terms[i]);
    }
}

void add_term_to_query(query* query, char* term) {
    if(query->current_size == query->max_size) {
        query->terms = realloc(query->terms, (query->max_size*2)*sizeof(char*));
    }
    query->terms[query->current_size] = term;
    query->current_size++;

}

void split_query_into_terms(char* querystr, query* query) {
    char* myquery = strdup(querystr);
    char* reentrant_saver;
    
    char* token;
    token = strtok_r(querystr, " \n", &reentrant_saver);
    while(token != NULL) {
        if(find_dict_entry(token)) {
            add_term_to_query(query, token);
        }
        token = strtok_r(NULL, " \n", &reentrant_saver);
    }
    free(myquery);
    print_query_struct(query);
}

void build_postingslist(dictionary_entry* dict_entry) {
    // return if we have already read the postingslist for this term
    if(dict_entry->posting != NULL) {
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
    free(entry);
    prev->next = NULL;
}

void handle_token(char* token) {
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

void score_query(query* query) {
    doc_score* doc_scores = NULL;

    int i;
    for(i = 0; i < query->current_size; i++) {
        dictionary_entry* dict_entry = find_dict_entry(query->terms[i]);
        postings_entry* posting = dict_entry->posting;
 
        int j;
        for(j = 0; j < dict_entry->occurences; j++) {
            doc_score* score = lookup_doc_score(doc_scores, posting->docId);
            if(score == NULL) {
                score = alloc_init_doc_score(query->current_size);
                score->docid = posting->docId;
            }
        }

    }
}

void search(char* querystr) {
    char* token;

    query* q = init_alloc_query();
    split_query_into_terms(querystr, q);
    int i;
    for(i=0; i < q->current_size; i++) {
        token = q->terms[i];
        handle_token(token);
    }
    score_query(q);
}

int main(int argc, char* argv[])
{
    build_dictionary("target/dictionary.txt");
    print_dictionary();
    search(argv[1]);
    print_dictionary();
}

