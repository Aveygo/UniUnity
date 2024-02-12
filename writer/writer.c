/*******************************************************************************
 * writer.c
 * Handles saving and encrypting student preferences to disk 
*******************************************************************************/

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

#include "writer.h"
#include "../compress/compress.h" /*unflatten flatten*/
#include "../student/student.h" /* sanity_check_students*/

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;
unsigned int enc_x; /* Seed of the stream cipher's PRNG */

/**
 * Shifts each bit to the left, used for encryption
 * Return: unsigned int
 *
 * Inputs
 *  - value     The integer bits to shift
 *  - shift     The amount to shift by 
 * Outputs
 *  - Integer with shifted bits
 * 
 */
unsigned int rol(unsigned int value, int shift) {
    return (value << shift) | (value >> (sizeof(value) * 8 - shift));
}

/**
 * Hash function with xor and bit shifting
 * Return: unsigned int
 *
 * Inputs
 *  - input     Input to hash
 * Outputs
 *  - Hashed integer 
 * 
 */
unsigned int hash(int input) { 
    unsigned int result = 0x55555555;

    for (int i=0; i<32; i++) {
        result ^= input++;
        result = rol(result, 5);
    }
    return result;
}

/**
 * Hash function for string, mainly for password
 * Return: unsigned int
 *
 * Inputs
 *  - text              String to hash
 *  - password_len      Length of the string  
 * Outputs
 *  - Hashed integer
 * 
 */
unsigned int hash_c(char* text, int password_len) {
    unsigned int result = 0xFFFFFFF;
    for (int i=0;i<password_len;i++) {
        result = result ^ hash(text[i]);
    }
    return result;
}

/**
 * File operations are buffered and for some reason 
 * break (not sure why tbh) when writing integers.
 * This function splits an integer into 4 chars.
 * Return: void
 *
 * Inputs
 *  - target        Integer to write
 *  - file          File to write to  
 * Outputs
 *  - 4 added bytes in file
 * 
 */
void write_int(int target, FILE* file) {
    /* Encrypt the integer using a stream cipher */
    enc_x = hash(enc_x);
    unsigned int enc = (unsigned int)target ^ enc_x;

    /* Split into chars */
    unsigned char bytes[4];
    bytes[0] = (enc >> 24) & 0xFF;
    bytes[1] = (enc >> 16) & 0xFF;
    bytes[2] = (enc >> 8) & 0xFF;
    bytes[3] = enc & 0xFF;

    /* Write chars into file */
    fwrite(&bytes, sizeof(unsigned char), 4, file);
}

/**
 * File contains 4 chars per integer. Here we
 * Recombine the chars back into a single integer. 
 * Return: int
 *
 * Inputs
 *  - file          File to read from
 * Outputs
 *  - Read 4 bytes, convert to single integer
 * 
 */
unsigned int read_int(FILE* file) {
    unsigned char buffer[4];
    unsigned int target;

    /* Read 4 chars from the file */
    fread(&buffer,sizeof(buffer),1,file);

    /* Convert the 4 chars into a single integer */
    target = ((unsigned int)buffer[0] << 24) |
            ((unsigned int)buffer[1] << 16) |
            ((unsigned int)buffer[2] << 8) |
            ((unsigned int)buffer[3]);

    /* Decrypt the integer using a stream cipher & return result */
    enc_x = hash(enc_x);
    unsigned int enc = (unsigned int)target ^ enc_x;
    return enc;
    
}


/**
 * Take an array of students struct and save to disk
 * Return: int, 0 success, 1 fail
 *
 * Inputs
 *  - students          Target array of student structs
 *  - num_students      Size of students array
 *  - password          Password for encryption
 *  - filename          File to write to
 * Outputs
 *  - File with compressed & encrypted student preferences 
 *    Returns 
 * 
 */
int save_students_bin(Student* students, int num_students, char password[], char filename[]) {
    /* Set the seed of the stream cipher's PRNG to the password */
    enc_x = hash_c(password, strlen(password));

    /* Try and open the file*/
    if (DEBUG) {printf("[DEBUG] Opening file...\n");}
    FILE *file = fopen(filename, "wb");

    if (file == NULL) {
        return 1;
    }

    /* Allocate memory and convert the students into a 1d array */ 
    int* int_student_arr = (int*)malloc(sizeof(int) * (1+MAX_STUDENT_PREFERENCES) * num_students);
    int int_student_arr_size = flatten(students, num_students, int_student_arr);

    unsigned int chunk_size = 512;
    unsigned int num_chunks = (int_student_arr_size + (chunk_size - 1)) / chunk_size;
    if (DEBUG) {printf("[DEBUG] Writing %d students as %d chunks\n", num_students, num_chunks);}

    /*
        Write the header
        SOF
        UniUnity                [8 bytes] Checks if password is correct when reading
        num_preferences          [4 bytes] The maximum number of preferences the student can have
        num_students            [4 bytes] Number of unique students
        1d_size                 [4 bytes] Size of 1d representation of students
        chunk_size              [4 bytes] Amount of students per chunk
        num_chunks              [4 bytes] Number of chunks in file

    */
    if (DEBUG) {printf("[DEBUG] Writing header\n");}
    write_int(1433299285,               file); /* UniU */
    write_int(1852404857,               file); /* nity */
    write_int(MAX_STUDENT_PREFERENCES,  file);
    write_int(num_students,             file);
    write_int(int_student_arr_size,     file);
    write_int(chunk_size,               file);
    write_int(num_chunks,               file);

    /* Start processing the chunks */
    for (int chunk_id=0; chunk_id<num_chunks; chunk_id++) {

        /* Allocate memory for a chunk and move students into it */
        int* student_chunk = (int*)malloc(sizeof(int) * chunk_size);
        int s;
        for (s =0; (s < chunk_size) && (chunk_id*chunk_size + s < int_student_arr_size); s++) {
            student_chunk[s] = int_student_arr[chunk_id*chunk_size + s];
        }
        if (DEBUG) {printf("[DEBUG] Using %d student in chunk %d\n", s, chunk_id);}

        /*
            printf("Chunk contains: ");
            for (int i=0; i<s; i++) {
                printf("%d ", student_chunk[i]);
            }
            printf("\n");
        */

        /* Get the unique student ids for computing frequencies */
        int num_unique;
        int * known_student_ids = find_unique(student_chunk, s, &num_unique);
        if (DEBUG) {printf("[DEBUG] Found %d unique students\n", num_unique);}

        /*
            printf("Unique students in chunk are: ");
            for (int i=0; i<num_unique; i++) {
                printf("%d ", known_student_ids[i]);
            }
            printf("\n");
        */

        /* 
            For the chunk of students, find and sort by frequency
            Define a pointer to start adding nodes too
            We will also use this pointer to add branches of the huffman tree later on.
            The number of branches = n-1, making the total = n + (n-1) or at most 2n    
        */
        if (DEBUG) {printf("[DEBUG] Computing frequencies\n");}
        node* frequencies = (node*)calloc(num_unique*num_unique, sizeof(node));
        build_initial_frequencies(frequencies, student_chunk, s, known_student_ids, num_unique);

        /* Construct a huffman tree from the student id frequencies */
        if (DEBUG) {printf("[DEBUG] Computing Tree\n");}
        node root;
        build_tree(&root, num_unique, frequencies);

        /* Compress the students */
        int top=0;
        int* compressed = (int*)malloc(sizeof(int) * (s * s * (1+MAX_STUDENT_PREFERENCES)));
        for (int i=0; i<s; i++){
            id_to_code(
                student_chunk[i], 
                root.child, 
                frequencies, 
                num_unique, 
                compressed, 
                &top
            );
        }

        if (DEBUG) {printf("[DEBUG] Students compressed to %d bits\n", top);}

        /*
            printf("Student bits: ");
            for (int i=0; i<top; i++) {
                printf("%d ", compressed[i]);
            }
            printf("\n");
        */

        /* Convert the tree into a file ready format */
        int* serialized_tree = (int*)calloc(s * s + 2, sizeof(int));
        int tree_serialized_size = 0;  
        serialize_tree(root.child, serialized_tree, &tree_serialized_size);
        tree_serialized_size = tree_serialized_size + 1;

        if (DEBUG) {printf("[DEBUG] Local tree serialized\n");}

        

        /* Pad the compressed format so we can convert it into ints */
        int amount_to_pad = ((top + (sizeof(int) * 8) - 1) / (sizeof(int) * 8)) * (sizeof(int) * 8) - top;
        if (DEBUG) {printf("[DEBUG] Compressed format needs %d bits of padding\n", amount_to_pad);}
        for (int i=0;i<amount_to_pad;i++){
            compressed[top] = 0;
            top = top + 1;
        }

        /* 
            Convert the sequence of ones and zeros to int array.
            The compressed format is an integer array of ones and zeros.
            However, we can optimize this by using the bits of an integer
            to represent this data instead.
            eg:
                compressed student      = 0 1 0 0 1 1 1 0 0 1 0 1
                "optimized" integer     = 1253
            
            The same data is stored, just in a single integer, rather than many. 
        */
        int num_compressed = (top + (sizeof(int)*8-1)) / (sizeof(int) * 8);
        unsigned int compressed_optimized[num_compressed];
        for (int i=0; i<num_compressed; i++) { compressed_optimized[i] = 0; }
        for (int i=0; i< top; i++) {
            /* Bit shifts are cool and are not that scary */
            compressed_optimized[i/(sizeof(int) * 8)] = compressed_optimized[i/(sizeof(int) * 8)] << 1;
            compressed_optimized[i/(sizeof(int) * 8)] += (unsigned int)compressed[i];
        }

        /* 
            Write the number of tree nodes and number of compressed student bits
            Start of chunk:
                tree_size       [4 bytes] The number of nodes in the tree
                students_size   [4 bytes] How many bits were used to compress the students
                chunk_size      [4 bytes] The number of students within a chunk
        */
        write_int(tree_serialized_size,     file);
        write_int(top,                      file);
        write_int(s,                        file);
        
        /* Write the compressed student array to file */
        if (DEBUG) {printf("[DEBUG] Loaded raw student data: ");}
        for (int i=0; i< num_compressed; i++) {
            write_int(compressed_optimized[i], file);
            if (DEBUG) {printf("%u ", compressed_optimized[i]);}
        }
        if (DEBUG) {printf("\n");}

        /* Write the serialized tree to file */ 
        for (int i=0; i< tree_serialized_size; i++) {
            write_int(serialized_tree[i], file);
            if (DEBUG) {printf("%u ", serialized_tree[i]);}
        }

        /* Free up memory used to write the chunk */
        free(frequencies);
        free(serialized_tree);
        free(compressed);
        free(student_chunk);
        free(known_student_ids);
    }

    /* Close file, free 1d representation, and return success*/
    fclose(file);
    free(int_student_arr);
    return 0;
}       

/**
 * Check if the password for a file is valid
 * Return: int, 0 success, >=1 fail
 *
 * Inputs
 *  - password     The text used to encrypt the file
 *  - filename     File to attempt to decrypt 
 * Outputs
 *  - Result csv file OR compressed student preferences
 * 
 */
int check_for_password( char password[], char filename[]) {
    /* Set the seed of the stream cipher's PRNG to the password */
    enc_x = hash_c(password, strlen(password));

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return 2;
    }

    /* Check if the password could decrypt at least some of the data */
    if (read_int(file) != 1433299285 || read_int(file) != 1852404857) {
        fclose(file);
        return 1;
    }

    /* Close and return success */
    fclose(file);
    return 0;
}

/**
 * Given a filename, load, decrypt, decompress, and return an 
 * array of students
 * Return: int, 0 success, >=1 fail
 *
 * Inputs
 *  - students                  Pointer to change to loaded students
 *  - result_num_students       Pointer to change to length of loaded students
 *  - password                  Password used to encrypt file
 *  - filename                  File to read from
 * Outputs
 *  - Free's any memory in original student's array and
 *    changes it to the newly loaded student preferences.
 * 
 */
int load_students_bin(Student ** students, int* result_num_students , char password[], char filename[]) {
    /* Set the seed of the stream cipher's PRNG to the password */
    enc_x = hash_c(password, strlen(password));

    /* See if we can open the file */
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return 1;
    }
    
    /* Check if the password was correct */
    if (read_int(file) != 1433299285 || read_int(file) != 1852404857) {
        fclose(file);
        return 2;
    }
    
    /* Check if the maximum number of preferences is the same as ours */
    if (read_int(file) != MAX_STUDENT_PREFERENCES) {
        printf("The executable that created the file to be read was compiled with a different number of allowed student preferences. Nothing to be done.\n");
        fclose(file);
        return 3;
    }
    
    /* 
        Read the rest of the header. 
        Used to allocate enough memory to store 
        tree nodes and the final result.
    */
    unsigned int num_students       = read_int(file);
    unsigned int student_1d_size    = read_int(file);
    unsigned int chunk_size         = read_int(file);
    unsigned int num_chunks         = read_int(file);
    
    if (DEBUG) {
        printf("[DEBUG] num_students %u\n", num_students);
        printf("[DEBUG] chunk_size %u\n", chunk_size);
        printf("[DEBUG] num_chunks %u\n", num_chunks);
    }

    /* Allocate memory for the 1d representation of students */
    int * students_1d = (int*) malloc(sizeof(int) * student_1d_size * 2);
    int loaded_students = 0;

    /* Proccess chunks in file */
    for (int chunk_id=0; chunk_id<num_chunks; chunk_id++) {
        
        /* Read in chunk data */
        int tree_serialized_size        = read_int(file);
        int compressed_students_size    = read_int(file);
        int num_student_in_chunk        = read_int(file);

        if (DEBUG) {
            printf("[DEBUG] Working on chunk %u/%u\n", chunk_id+1, num_chunks);
            printf("[DEBUG] tree_serialized_size %u\n", tree_serialized_size);
            printf("[DEBUG] compressed_students_size %u\n", compressed_students_size);
            printf("[DEBUG] num_student_in_chunk %u\n", num_student_in_chunk);
            

            /* Uncomment to step through decompression chunks */
            /*
                printf("Press enter to decompress chunk...\n");
                int enter = 0;
                while (enter != '\r' && enter != '\n') { enter = getchar(); } 
            */
        }
        
        /* Allocate memory for an integer array that will contain the compression bits */
        int num_compressed = (compressed_students_size + (sizeof(int)*8-1)) / (sizeof(int) * 8);
        int* compressed_students = (int*) malloc(sizeof(int) * compressed_students_size * 2);
        int current_students = 0;

        for (int i=0; i<num_compressed; i++) {
            int student_optimized = read_int(file);
            
            /* 
                Convert the optimized integers into bits
                eg:
                    "optimized" integer     = 1253
                    compressed student      = 0 1 0 0 1 1 1 0 0 1 0 1

            */
            unsigned int student_bits [sizeof(unsigned int) * 8];

            /*
                Read backwards, building the optimized integer 
            */
            for (int i = sizeof(unsigned int) * 8 - 1; i >= 0; i--) {
                student_bits[i] = student_optimized & 1;
                student_optimized = student_optimized >> 1;
            }

            /*
                Save the optimized integer into the array,
                while also making sure we do not process any padding.
            */
            for (int i=0; i<sizeof(unsigned int) * 8;i++) {
                if (current_students < compressed_students_size) {
                    compressed_students[current_students] = student_bits[i];
                    current_students = current_students + 1;
                }
            }
        }

        if (DEBUG) {printf("[DEBUG] Loaded %d student bits\n", current_students);}

        /*
            printf("Optimized Students: ");
            for (int i=0; i< compressed_students_size; i++) {
                printf("%d ", compressed_students[i]);
            }
            printf("\n");
        */

        /* Read the serialized tree into memory */
        int* tree_serialized = (int*) malloc(sizeof(int) * (tree_serialized_size * 2));
        for (int i=0; i<tree_serialized_size; i++) {
            tree_serialized[i] = read_int(file);
        }
        
        /* Allocate enough memory to store all the nodes of the tree */
        node* nodes = (node*) malloc(sizeof(node) * (tree_serialized_size * 4));
        
        /* Convert the serialized tree into a struture we can use */
        int top = 0;
        node* root = deserialize_tree(tree_serialized, nodes, &top);
        if (DEBUG) {printf("[DEBUG] Built tree\n");}

        /* Finally, decompress the student data */
        int* ids = (int*) malloc(sizeof(int) * num_student_in_chunk*2);
        int num_ids = codes_to_ids(ids, compressed_students, compressed_students_size, root);

        /* if (DEBUG) {print_tree(root, 0);} */

        /* Save the decompressed students into final array */
        for (int i=0; i<num_ids;i++) {  
            students_1d[loaded_students + i] = ids[i];
        }
        loaded_students += num_student_in_chunk;

        /* Free any memory associated with loading in chunk */
        free(nodes);
        free(compressed_students);
        free(ids);
        free(tree_serialized);

    }
    
    /* Allocate memory for the result */
    Student * result_students = (Student*) calloc(num_students, sizeof(Student));

    if (result_students == NULL) {
        printf("FAILED TO ALLOCATE!\n");
        return 4;
    }

    /* Free any memory that may have been accidentally provided */
    if (*students != NULL) {
        free(*students);
    }

    /* Point to our result */
    *students = result_students;

    /* Convert the 1d representation of our student into an array of student structs */
    unflatten(students_1d, num_students, result_students);

    /* Free the 1d representation as the data is now store in result_students */
    free(students_1d);

    /* Update the output number of loaded students */
    *result_num_students = num_students;
    
    /* Check if the students file seemed valid, mainly for debugging */
    if (sanity_check_students(result_students, num_students) == 0) {
        fclose(file);
        return 5;
    }

    /* Close and return success */
    fclose(file);
    return 0;
}