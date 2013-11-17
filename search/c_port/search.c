#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "uthash/uthash.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define DEFAULT_TERM_NUM 5
#define DEBUG 0
#define CONFIG_PORT 32003

#define MAXOUTPUTSIZE 1024
#define MAXBUFLEN 128

typedef struct postings_entry postings_entry;
long long wall_clock_time();
static int N= 0;

typedef struct {
    char* word;
    uint32_t byte_offset;
    uint32_t occurences;
    uint32_t occurences_abstract;
    postings_entry* posting;
    UT_hash_handle hhd;         /* makes this structure hashable */
} dictionary_entry;

/* query has a number of terms (strings) */
typedef struct {
    char* term;
    float score;
    UT_hash_handle hh;
    dictionary_entry* dict_entry;
} query;

typedef struct {
    int docid;
    float score;
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
    HASH_ADD_KEYPTR(hhd, dictionary, dict_entry->word, strlen(dict_entry->word), dict_entry);
}

dictionary_entry* find_dict_entry(char* word) {
    dictionary_entry* dict_entry;
    HASH_FIND(hhd, dictionary, word, strlen(word), dict_entry);
    if(dict_entry == NULL)
        printf("dictionary lookup: %s failed\n", word);
    return dict_entry;
}

void init_alloc_postings_entry(postings_entry** entry) {
    *entry = malloc(sizeof(postings_entry));
    (*entry)->next = NULL;
}

void init_alloc_query(query** q) {
    *q = malloc(sizeof(query));
    (*q)->term = NULL;
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
    char* buffer = malloc(sizeof(char)*128);
    while(!feof(f)) {
        dictionary_entry* dict_entry = init_alloc_dictionary_entry();

//        fscanf(f, "%s %" SCNu32 " %" SCNu32 " %" SCNu32 "\n", buffer, &dict_entry->byte_offset,
//               &dict_entry->occurences, &dict_entry->occurences_abstract);
        fscanf(f, "%s %" SCNu32 " %" SCNu32 "\n", buffer, &dict_entry->byte_offset,
               &dict_entry->occurences);

        dict_entry->word = strdup(buffer);
        hash_dict_entry(dict_entry);
    }
    fclose(f);
    free(buffer);
}

void print_dictionary() {
    dictionary_entry *dict_entry;

    for(dict_entry=dictionary; dict_entry != NULL; dict_entry=dict_entry->hhd.next) {
        if(dict_entry->posting == NULL) {
            //continue;
        }
//        printf("%s %" SCNu32 " %" SCNu32 " %" SCNu32 "\n", dict_entry->word, dict_entry->byte_offset,
//                dict_entry->occurences, dict_entry->occurences_abstract);
        printf("%s %" SCNu32 " %" SCNu32 "\n", dict_entry->word, dict_entry->byte_offset,
                dict_entry->occurences);

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

void hash_query_entry(query** query_dict, query* myq) {
    HASH_ADD_KEYPTR(hh, *query_dict, myq->term, strlen(myq->term), myq);
    if(DEBUG)
        printf("hashed q entry: key:%s leng:%f\n", myq->term, myq->score);
}

void add_term_to_query(query** query_dict, char* term, dictionary_entry* dict_entry) {
    // if term in query already, increase score:
    query* myq = NULL;

    HASH_FIND(hh, *query_dict, term, strlen(term), myq);
    if(!myq) {
        init_alloc_query(&myq);
        myq->term = term;
        myq->score = 1.0f;
        myq->dict_entry = dict_entry;
        hash_query_entry(query_dict, myq);
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

void split_query_into_terms(query** query_dict, char* querystr) {
    char* reentrant_saver;

    char* token;
    token = strtok_r(querystr, " \n\r\t", &reentrant_saver);

    while(token != NULL) {
        dictionary_entry* dict_entry = find_dict_entry(token);

        if(dict_entry != NULL) {
            if(DEBUG)
                printf("found token in dictionary: %s\n", token);
            char* copy = strdup(token);
            add_term_to_query(query_dict, copy, dict_entry);
        }
        token = strtok_r(NULL, " \n\r\t", &reentrant_saver);
    }
}

void print_query_struct_str(query** query_dict, char* target) {
    query* entry;
    dictionary_entry* current_dict_entry;
    int charcounter = 0;
    for(entry=*query_dict; entry != NULL; entry=entry->hh.next) {
        current_dict_entry = find_dict_entry(entry->term);
        charcounter += snprintf(target+charcounter, MAXOUTPUTSIZE-charcounter, "%s: %f\n\t", entry->term, entry->score);
        postings_entry* postentry = current_dict_entry->posting;
        while(postentry) {
            charcounter += snprintf(target+charcounter, MAXOUTPUTSIZE-charcounter, "%d ", postentry->docId);
            postentry = postentry->next;
        }
        charcounter += snprintf(target+charcounter, MAXOUTPUTSIZE-charcounter, "\n\t");
    }

}

void print_query_struct(query** query_dict) {
    query* entry;
    printf("query_dict:\n");
    dictionary_entry* current_dict_entry;
    for(entry=*query_dict; entry != NULL; entry=entry->hh.next) {
        current_dict_entry = find_dict_entry(entry->term);
        printf("\n\t%s: %f\n\t", entry->term, entry->score);
        postings_entry* postentry = current_dict_entry->posting;
        while(postentry) {
            printf("%d ", postentry->docId);
            postentry = postentry->next;
        }
    }
    printf("\n\n");
}

void build_postingslist(char* token) {
    dictionary_entry* dict_entry = find_dict_entry(token);
    if(!dict_entry) {
        return;
    }

    // return if we have already read the postingslist for this term
    if(dict_entry->posting) {
        return;
    }
    // if there is no postings list to be read, return
    if(dict_entry->occurences == 0) {
        return;
    }

    FILE* f_postings = fopen(postings_file, "rb");
    fseek(f_postings, dict_entry->byte_offset, SEEK_SET); //SEEK_SET is offset from beginning of file

    postings_entry* postentry;
    init_alloc_postings_entry(&postentry);
    dict_entry->posting = postentry;
    postings_entry* prev;

    if(DEBUG)
        printf("build postlist for token: %s, times: %"SCNu32"\n", token, dict_entry->occurences);

    uint32_t i = 0;
    for(i = 0; i < dict_entry->occurences; i++) {
        fread(&(postentry->docId), 8, 1, f_postings);
        //fread(&(postentry->term_frequency), 4, 1, f_postings);

        prev = postentry;
        postings_entry* tmp;
        init_alloc_postings_entry(&tmp);
        postentry->next = tmp;
        postentry = tmp;

    }
    //free(entry); // breaks all
    prev->next = NULL;
    fclose(f_postings);

}

void delete_query_struct(query** query_dict) {
    query* entry = NULL;
    query* tmp = NULL;
    HASH_ITER(hh, *query_dict, entry, tmp) {
        HASH_DEL(*query_dict, entry);
        free(entry->term);
        free(entry);
    }
}

void handle_token(char* token) {
    if(DEBUG) {
        printf("handle token: %s\n", token);
    }
    char* copy = strdup(token);
    build_postingslist(copy);
    free(copy);
}

void add_doc_score(doc_score** doc_scores, doc_score* doc_score_to_add) {
    HASH_ADD_INT(*doc_scores, docid, doc_score_to_add);
}

void alloc_init_doc_score(doc_score** score, int query_terms) {
    *score = malloc(sizeof(doc_score));
}

doc_score* lookup_doc_score(doc_score** doc_score_hash, int docid) {
    doc_score* score = NULL;
    HASH_FIND_INT(*doc_score_hash, &docid, score);
    return score;
}

void destroy_doc_score(doc_score* score) {
    free(score);
}

int doc_score_sort(void* a, void* b) {
    doc_score* score_a = (doc_score*)a;
    doc_score* score_b = (doc_score*)b;
    if(score_a->score < score_b->score) {
        return 1;
    }
    if(score_a->score > score_b->score) {
        return -1;
    }
    return 0;
}

void print_doc_score_str(doc_score** doc_scores, char* target) {
    doc_score* element = NULL;
    doc_score* tmp = NULL;
    int printsize = 0;
    HASH_ITER(hh, *doc_scores, element, tmp) {
        if(printsize < MAXOUTPUTSIZE-24) {
            printsize += snprintf(target+printsize, MAXOUTPUTSIZE-printsize, "%d %f\n", element->docid, element->score);
        }
    }
}

void print_doc_scoring(doc_score** doc_scores) {
    doc_score* element = NULL;
    doc_score* tmp = NULL;
    printf("Document scores:\n");
    HASH_ITER(hh, *doc_scores, element, tmp) {
        printf("\t%d: %f\n", element->docid, element->score);
    }
}

void score_query(query** query_dict, char* target_str) {
    doc_score* doc_scores = NULL;

    query* entry;
    float squared_sum_of_weights = 0.0f;
    for(entry=*query_dict; entry != NULL; entry=entry->hh.next) {

        dictionary_entry* dict_entry = entry->dict_entry;
        postings_entry* posting = dict_entry->posting;

        float idf_term = N/(dict_entry->occurences+1);
        idf_term = logf(idf_term);

        int j;
        for(j = 0; j < dict_entry->occurences; j++) {
            doc_score* score = lookup_doc_score(&doc_scores, posting->docId);
            float weight = 1.0f+logf(posting->term_frequency);
            weight = weight*idf_term;
            squared_sum_of_weights+=weight*weight;
            if(!score) {
                alloc_init_doc_score(&score, 1);
                score->docid = posting->docId;
                score->score = weight;
                add_doc_score(&doc_scores, score);
            } else {
                score->score += weight;
            }
            posting = posting->next;
        }
    }
    squared_sum_of_weights = sqrtf(squared_sum_of_weights);
    squared_sum_of_weights = 1/squared_sum_of_weights;

    doc_score* element = NULL;
    doc_score* tmp = NULL;
    HASH_ITER(hh, doc_scores, element, tmp) {
        element->score = element->score*squared_sum_of_weights;
    }

    HASH_SORT(doc_scores, doc_score_sort);
    print_doc_score_str(&doc_scores, target_str);

    //cleanup
    element = NULL;
    tmp = NULL;
    HASH_ITER(hh, doc_scores, element, tmp) {
        HASH_DEL(doc_scores, element);
        destroy_doc_score(element);
    }
}

void prefetch_tokens(query** query_dict) {
    query* entry;

    for(entry=*query_dict; entry != NULL; entry=entry->hh.next) {
        handle_token(entry->term);
    }

}

void doSearch(char* querystr, char* result_target) {
    query* query_dict = NULL;

    if(DEBUG)
        printf("given query: %s\n", querystr);

    split_query_into_terms(&query_dict, querystr);

    prefetch_tokens(&query_dict);

    score_query(&query_dict, result_target);

    delete_query_struct(&query_dict);

}

int isPortSet(int port){
    if(port == 0){
        return 0;
    } else {
        return 1;
    }
}

void startLocalServer(int given_port){
    char* configfile = "config.txt";
    char configbuffer[16];

    FILE* fconfig = fopen(configfile, "r");
    fgets(configbuffer, 16, fconfig);

    int sockfd;
    ssize_t n;

    struct sockaddr_in my_addr, load_dist_addr, useraddr;
    socklen_t len;

    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    if (isPortSet(given_port)) {
        bzero(&my_addr,sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_addr.s_addr = inet_addr(configbuffer);
        my_addr.sin_port=htons(given_port);
        printf("Starting server on ip: %s:%d\n", configbuffer, given_port);
    } else {
        bzero(&my_addr,sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_addr.s_addr = inet_addr(configbuffer);
        my_addr.sin_port=htons(CONFIG_PORT);
        printf("Starting server on ip: %s:%d\n", configbuffer, CONFIG_PORT);
    }

    bzero(&useraddr,sizeof(useraddr));
    useraddr.sin_family = AF_INET;

    bind(sockfd,(struct sockaddr *)&my_addr,sizeof(my_addr));

    printf("starting server...\n");

    typedef struct {
        __uint32_t ip;
        __uint16_t port;
        char msg[MAXBUFLEN];
    } payload;

    payload p;
    char* strbuffer = malloc(MAXOUTPUTSIZE);

    printf("Ready for query...\n");
    for(;;)
    {

        bzero(&p,sizeof(p));
        len = sizeof(load_dist_addr);

        n = recvfrom(sockfd,&p,sizeof(p),0,(struct sockaddr *)&load_dist_addr,&len);

        useraddr.sin_addr.s_addr = p.ip;
        useraddr.sin_port = htons(32000);
        //useraddr.sin_port = p.port;

        // printf("port: %hu, ip: %s\n",ntohs(p.port),inet_ntoa(useraddr.sin_addr));
        // printf("query: %s\n",p.msg);


        //We have a query, do the search

        // printf("-------------------------------------------------------\n");
        // printf("executing query: %s\n",p.msg);
        // printf("-------------------------------------------------------\n");

        long long before = wall_clock_time();

        char* querystr = strdup(p.msg);
        doSearch(querystr, strbuffer);

        if(DEBUG) {
            printf("%s", strbuffer);
        }


        // printf("Done\n");
        // printf("Sending answer...\n");

        free(querystr);

        // long long after = wall_clock_time();
        // long long el = after-before;
        // float mytime = (float)el/1000000000.0;
        // printf("ELAPSED: %f s\n\n", mytime);

        // printf("Sending string:\n%s\n", strbuffer);
        int sentbytes = sendto(sockfd, strbuffer, strlen(strbuffer), 0, (struct sockaddr *)&useraddr, sizeof(useraddr));
        // printf("Done!");
    }

    free(strbuffer);
    close(sockfd);
}

int main(int argc, char* argv[])
{
    build_dictionary("target/dictionary.txt");
    N = find_dict_entry("*")->occurences;
    printf("Docs in collection: %d\n", N);
    if(argc > 2) {

        char* searchstr = strdup(argv[2]);
        char* result = malloc(MAXOUTPUTSIZE);

        long long before = wall_clock_time();

        doSearch(searchstr, result);

        free(searchstr);

        long long after = wall_clock_time();
        long long el = after-before;
        float mytime = (float)el/1000000000.0;

        printf("ELAPSED doSearch: %f s\n\n", mytime);
        if(DEBUG) {
            printf("%s", result);
        }
        free(result);
    } 
    else if (argc > 1)
    {
        int port = atoi(argv[1]);
        startLocalServer(port);
    }

    else {
        startLocalServer(0);
    }
    print_dictionary();
}


long long wall_clock_time()
{
#ifdef __linux__
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (long long)(tp.tv_nsec + (long long)tp.tv_sec * 1000000000ll);
#else
#warning "Your timer resoultion might be too low. Compile on Linux and link with librt"
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_usec * 1000 + (long long)tv.tv_sec * 1000000000ll);
#endif
}

