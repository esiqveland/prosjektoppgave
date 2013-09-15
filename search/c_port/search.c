#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "uthash/uthash.h"

typedef struct {
	char* word;
	uint32_t byte_offset;
	uint32_t occurences;
	uint32_t occurences_abstract;
} dictionary_entry;



void hash_dict_entry(dictionary_entry *dict_entry)
{
    fprintf(stdout, "%s %" SCNu32 " %" SCNu32 " %" SCNu32 "\n", dict_entry->word, dict_entry->byte_offset,
           dict_entry->occurences, dict_entry->occurences_abstract);

}

// Reads the dictionary into memory
void build_dictionary(const char* input_file_dict)
{
    FILE* f = fopen(input_file_dict,"r");
    
    char* buf = malloc(sizeof(char)*256);
    
    while(!feof(f)) {
    	dictionary_entry* dict_entry = malloc(sizeof(dictionary_entry));

    	dict_entry->word = malloc(sizeof(char)*64);
    	fscanf(f, "%s %" SCNu32 " %" SCNu32 " %" SCNu32 "\n", dict_entry->word, &dict_entry->byte_offset,
               &dict_entry->occurences, &dict_entry->occurences_abstract);
        
        hash_dict_entry(dict_entry);
        
    }
    fclose(f);
}

int main(int argc, char** argv)
{
    build_dictionary("target/dictionary.txt");
}
