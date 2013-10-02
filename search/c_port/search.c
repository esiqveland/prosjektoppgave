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


#define DEFAULT_TERM_NUM 5
#define DEBUG 1

#define SERVERIP "192.168.0.100"
#define MYPORT "4950"    // the port users will be connecting to
#define SERVERPORT "4951"

#define MAXBUFLEN 128

typedef struct postings_entry postings_entry;

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
    UT_hash_handle hhq;
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
    dictionary_entry *dict_entry;
    
    for(dict_entry=dictionary; dict_entry != NULL; dict_entry=dict_entry->hhd.next) {
        if(dict_entry->posting == NULL) {
            continue;
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

void hash_query_entry(query** query_dict, query* myq) {
    HASH_ADD_KEYPTR(hhq, *query_dict, myq->term, strlen(myq->term), myq);
    if(DEBUG)
        printf("hashed q entry: key:%s leng:%f\n", myq->term, myq->score);
}

void add_term_to_query(query** query_dict, char* term) {
    // if term in query already, increase score:
    query* myq = NULL;

    HASH_FIND(hhq, *query_dict, term, strlen(term), myq);
    if(!myq) {
        init_alloc_query(&myq);
        myq->term = term;
        myq->score = 1.0f;
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
    char* myquery = strdup(querystr);

    char* reentrant_saver;
    
    char* token;
    token = strtok_r(myquery, " \n\r\t", &reentrant_saver);
    
    while(token != NULL) {
        dictionary_entry* dict_entry = find_dict_entry(token);
        
        if(dict_entry != NULL) {
            if(DEBUG)
                printf("found token in dictionary: %s\n", token);
            add_term_to_query(query_dict, token);
        }
        token = strtok_r(NULL, " \n\r\t", &reentrant_saver);
    }
    free(myquery);
}

void print_query_struct(query** query_dict) {
    query* entry;
    printf("query_dict:\n");
    for(entry=*query_dict; entry != NULL; entry=entry->hhq.next) {
        printf("\t%s: %f\n", entry->term, entry->score);
    }
}

void build_postingslist(dictionary_entry* dict_entry) {
    // return if we have already read the postingslist for this term
    if(dict_entry->posting != NULL) {
        return;
    }
    if(dict_entry->occurences == 0) {
        return;
    }
    
    FILE* f_postings = fopen(postings_file, "rb");
    fseek(f_postings, dict_entry->byte_offset, SEEK_SET); //SEEK_SET is offset from beginning of file
    
    postings_entry* entry;
    init_alloc_postings_entry(&entry);
    dict_entry->posting = entry;
    postings_entry* prev;

    int i;
    for(i = 0; i < dict_entry->occurences; i++) {
        fread(entry, 8, 1, f_postings);
        init_alloc_postings_entry(&(entry->next));
        prev = entry;
        entry = entry->next;
    }
    //free(entry); // breaks all
    prev->next = NULL;
    fclose(f_postings);
}

void handle_token(char* token) {
    if(DEBUG) {
        printf("handle token: %s\n", token);
    }
    dictionary_entry* dict_entry = find_dict_entry(token);
    if(dict_entry) {
        build_postingslist(dict_entry);
    }
}

void add_doc_score(doc_score** doc_scores, doc_score* doc_score_to_add) {
    HASH_ADD_INT(*doc_scores, docid, doc_score_to_add);
}

void alloc_init_doc_score(doc_score** score, int query_terms) {
    *score = malloc(sizeof(doc_score));
    (*score)->scores = malloc(sizeof(float)*query_terms);
    int i;
    for(i = 0; i < query_terms; i++) {
        (*score)->scores[i] = 0.0f;
    }
}

doc_score* lookup_doc_score(doc_score** doc_score_hash, int docid) {
    doc_score* score;
    HASH_FIND_INT(*doc_score_hash, &docid, score);
    return score;
}

void destroy_doc_score(doc_score* score) {
    if(score->scores != NULL)
        free(score->scores);
    score->scores = NULL;
}

void score_query(query** query_dict) {
    doc_score* doc_scores = NULL;

    query* entry;
    for(entry=*query_dict; entry != NULL; entry=entry->hhq.next) {
        
        dictionary_entry* dict_entry = find_dict_entry(entry->term);
        if(!dict_entry) {
            printf("could not find dict_entry for: %s\n", entry->term);
        }
        postings_entry* posting = dict_entry->posting;
 
        int j;
        for(j = 0; j < dict_entry->occurences; j++) {
            doc_score* score = lookup_doc_score(&doc_scores, posting->docId);
            if(!score) {
                alloc_init_doc_score(&score, 1);
                score->docid = posting->docId;
                add_doc_score(&doc_scores, score);
            }
        }

    }
}

void prefetch_tokens(query** query_dict) {
    query* entry;

    for(entry=*query_dict; entry != NULL; entry=entry->hhq.next) {
        handle_token(entry->term);
    }
}

void doSearch(char* querystr) {
    
    if(DEBUG)
        printf("given query: %s\n", querystr);
    query* query_dict = NULL;
    
    split_query_into_terms(&query_dict, querystr);

    print_query_struct(&query_dict);
    prefetch_tokens(&query_dict);    
    score_query(&query_dict);
}

void startLocalServer(){
    int i,n;
    int sockfd;
    
    struct sockaddr_in servaddr, cliaddr, useraddr;
    socklen_t len;
    
    char mesg[MAXBUFLEN];
    
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port=htons(32001);
    
    bzero(&useraddr,sizeof(useraddr));
    useraddr.sin_family = AF_INET;
    
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
    printf("starting server...\n");
    
    typedef struct {
        __uint32_t ip;
        __uint16_t port;
        char msg[MAXBUFLEN];
    } payload;
    
    payload p;
    
    for(;;)
    {
        printf("Ready for query...\n");
        bzero(&p,sizeof(p));
        len = sizeof(cliaddr);
        n = recvfrom(sockfd,&p,sizeof(p),0,(struct sockaddr *)&cliaddr,&len);
        
        useraddr.sin_addr.s_addr = p.ip;
        useraddr.sin_port = p.port;
        
        printf("port: %hu, ip: %u\n",p.port,p.ip);
        printf("query: %s\n",p.msg);
        
        //We have a query, do the search
        
        printf("-------------------------------------------------------\n");
        printf("executing query: %s\n",p.msg);
        printf("-------------------------------------------------------\n");
        
        doSearch(p.msg);
        
        print_dictionary();
        
        printf("Done\n");
        
    }
    
    close(sockfd);
}

int main(int argc, char* argv[])
{
    build_dictionary("target/dictionary.txt");
    if(argc > 1) {
        doSearch(argv[1]);
        print_dictionary();
    } else {
        startLocalServer();
    }
    print_dictionary();
}

