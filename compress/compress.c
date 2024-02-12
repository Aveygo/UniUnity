/*******************************************************************************
 * compress.c
 * Made for compressing a 1d representation of students
*******************************************************************************/

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

#include "compress.h"
#include "../utils/utils.h"

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;

/**
 * Convert the students into a flat/1d array of ids.
 * Return: int, size of the new array
 *
 * Inputs
 *  - students          Array of students to convert from
 *  - num_students      Number of students in array
 *  - int_student_arr   Array of output ints 
 * Outputs
 *  - Populated int_student_arr of student ids and their
 *    preferences
 * 
 */
int flatten(Student* students, int num_students, int* int_student_arr) {

    int int_student_arr_size = 0;
    
    /* Populate the array be reading each student */
    for (int i=0; i<num_students; i++) {

        /* Add the student's student id */
        int_student_arr[int_student_arr_size] = students[i].student_id;
        int_student_arr_size = int_student_arr_size + 1;

        /* Add student's preferences */
        for (int j=0; j<MAX_STUDENT_PREFERENCES; j++) {
            int_student_arr[int_student_arr_size] = students[i].preferences[j];
            int_student_arr_size = int_student_arr_size + 1;
        }
    }

    return int_student_arr_size;
}

/**
 * "inverse" of flatten, populates students array
 * Return: void
 *
 * Inputs
 *  - students_1d       Array of students ids to convert from
 *  - num_students      Number of students in 1d array
 *  - result            Array of output students
 * Outputs
 *  - Populated students array
 * 
 */
void unflatten(int* students_1d, int num_students, Student * result) {

    /* We will end up with num_students students */
    for (int i=0; i<num_students; i++) {

        /* 
            Each student is represented as 1 + the number of
            preferences they have.
        */

        /* Read the student id */
        result[i].student_id = students_1d[i * (MAX_STUDENT_PREFERENCES + 1)];
        result[i].preferences_size = 0;

        /* Populate the preferences */
        for (int j=0; j<MAX_STUDENT_PREFERENCES; j++) {
            int student_pref = students_1d[i*(MAX_STUDENT_PREFERENCES + 1) + j + 1];

            result[i].preferences[j] = student_pref;

            /* Empty preferences are the INT_MAX integer */
            if ( student_pref != INT_MAX) {    
                result[i].preferences_size = result[i].preferences_size + 1; 
            }
        }
    }
}

/**
 * Find all unique student ids in an array
 * Return: integer array
 *
 * Inputs
 *  - int_student_arr           Array of students ids to convert from
 *  - int_student_arr_size      Number of students in 1d array
 *  - num_unique                Pointer to the number of found unique students 
 * Outputs
 *  - pointer to array of unique student ids
 * 
 */
int* find_unique(int* int_student_arr, int int_student_arr_size, int* num_unique) {

    int* unique_student_ids = (int*)malloc(int_student_arr_size * sizeof(int));
    *num_unique = 0; 

    /* For each student */
    for (int i = 0; i < int_student_arr_size; i++) {
        
        /* Check if they are already in the list*/
        int found = 0;
        for (int j = 0; j < int_student_arr_size; j++) {
            if (int_student_arr[i] == unique_student_ids[j]) {
                found = 1;
            }
        }

        /* If they were not found, then add them to the unique list */
        if (found == 0){
            unique_student_ids[*num_unique] = int_student_arr[i];
            *num_unique = *num_unique + 1;
        }
    }

    /*
        printf("Unique students in chunk are: ");
        for (int i=0; i<*num_unique; i++) {
            printf("%d ", unique_student_ids[i]);
        }
        printf("\n");
    */

    return unique_student_ids;
}

/**
 * Find unique student ids (or empty preferences) and 
 * compute their frequencies as an array of nodes.
 * Return: node array
 *
 * Inputs
 *  - frequencies               Array of nodes for results
 *  - int_student_arr           Array of 1d student representation
 *  - int_student_arr_size      Number of integers in int_student_arr
 *  - known_student_ids         Array of known student ids
 *  - num_students              Number of integers in int_student_arr_size
 * Outputs
 *  - Populates frequencies array with initial nodes  
 * 
 */
node* build_initial_frequencies(node* frequencies, int* int_student_arr, int int_student_arr_size, int* known_student_ids, int num_students) {

    /* Sort the students for a search speed boost */
    /*sort(int_student_arr, int_student_arr_size);*/

    if (DEBUG) {
        printf("[DEBUG] Known student ids: ");
        for (int i = 0; i < num_students; i++) {
            printf("%d ", known_student_ids[i]);
        }
        printf("\n");
    }

    /* Populate node tree with the unique values, sorted by id */
    for (int i=0; i<num_students; i++) {
        frequencies[i].frequency = 0;
        frequencies[i].id = known_student_ids[i];
        frequencies[i].is_branch = 0;
    }

    if (DEBUG) {printf("[DEBUG] Populated frequencies");}

    /* Calculate the frequencies */
    for (int l=0; l<int_student_arr_size; l++) {

        int node_index = simple_search(int_student_arr[l], known_student_ids, num_students);

        if (node_index != -1){
            /* Found the student, increment the total count of them */
            frequencies[node_index].frequency++;
        } else {
            printf("Unknown student in chunk?: %d\n", int_student_arr[l]);
        }
    }

    return frequencies;
}

/**
 * Used by qsort to sort the array of nodes in order
 * by frequency
 * Return int
 * 
 * Inputs
 *  - a     pointer to a node
 *  - b     pointer to a node
 * Outputs
 *  - calculates the difference in frequency
 * 
 */
int cmpfunc(const void * a, const void * b) {
    node* node1 = (node*)a;
    node* node2 = (node*)b;    
    return node2->frequency - node1->frequency;
}

/**
 * Mainly for debugging
 * Return void
 * 
 * Inputs
 *  - root     pointer the node to print from
 *  - depth    What layer of the tree the node is
 * Outputs
 *  -  tree representation in stdout 
 * 
 */
void print_tree(node *root, int depth) {

    for (int i=0; i<depth; i++) {
        printf("\t");
    }

    if (root->is_branch == 1) {

        if (root->frequency == 0){
            /* Only the root has a frequency of 0. */
            printf("ROOT\n");
        } else {
            /* Show the branch and it's frequency */
            printf("[%d] Branch\n", root->frequency);
        }

        /* "Expand" the tree by exploring left, right, and children */
        if (root->left){
            print_tree(root->left, depth+1);
        }

        if (root->right) {
            print_tree(root->right, depth+1);
        }
        
        if (root->child) {
            print_tree(root->child, depth);
        }

    } else {

        /* Show the node and it's frequency */
        printf("[%d] Node %d", root->frequency, root->id);
        printf("\n");

        /* Node may still have a child if it was not processed */
        if (root->child != NULL) {
            print_tree(root->child, depth);
        }
    }
}


/**
 * Creates the huffman tree for compression
 * Return void
 * 
 * Inputs
 *  - top               Pointer to node that enters the tree
 *  - num_students      The number of initial students ("nodes") 
 *  - frequencies       Used to store data about the tree nodes, 
 *                      branches, and frequencies. Should initially
 *                      contain nodes with counts of unique students
 * Outputs
 *  -  Changes top to point to the beginning of the tree 
 * 
 */
void build_tree(node * top, int num_students, node* frequencies) {
    
    qsort(frequencies, num_students, sizeof(node), cmpfunc);

    if (DEBUG) {
        printf("[DEBUG] Student id frequencies: ");
        for (int i=0; i<num_students; i++) {
            printf("%d [%d], ", frequencies[i].id, frequencies[i].frequency);
        }
        printf("\n");
    }

    /* 
        NOTE
        Tree is constructed using a "doubly linked list" style. This makes traversal way
        easier, but is very easy to corrupt memory if not careful.
        Try not to touch. or look at.
    */

    /*
        Setup the parent of the first node, 
        this will be our entry into the tree and
        indicate when to stop building the tree
    */
    (*top).parent = NULL;
    (*top).is_branch = 1;
    (*top).child = &frequencies[0];    
    (*top).left = NULL; (*top).right = NULL;
    (*top).frequency = 0;
    frequencies[0].parent = top;

    /* Build the double pointers of each node */
    node* tail = &frequencies[0];
    
    for (int i=1; i<num_students; i++) {
        frequencies[i].parent = tail;
        tail = &frequencies[i];
        frequencies[i-1].child = tail;
    }

    /*
        We have a structure that currently looks like this: 
        top <=> node1 <=> ... <=> noden
                                    ^
                                    tail

        Where the nodes are stored in frequencies, and top
        was allocated outside of this function.
    */

    int num_frequencies = num_students;

    if (DEBUG) {printf("[DEBUG] Building the tree...\n");}

    while (tail->parent != top) {

        /* Create a branch to contain the lowest two nodes */
        num_frequencies = num_frequencies + 1;
        node* new_branch = &frequencies[num_frequencies-1];
        new_branch->is_branch = 1;
    
        /* Set the branch frequency to the sum of the last two nodes */
        new_branch->frequency = tail->frequency + tail->parent->frequency;
        
        /* Set the left/right pointer to the smaller/larger of the two nodes respectively */
        if (tail->frequency > tail->parent->frequency) {
            new_branch->right = tail;
            new_branch->left = tail->parent;
        } else {
            new_branch->left = tail;
            new_branch->right = tail->parent;
        }

        if (DEBUG) {printf("[DEBUG] Created a new branch with frequency: %d\n", new_branch->frequency);}

        /*
            Here we calculate where the branch should be inserted into our double linked list
            We do this by starting from the bottom, and "bubbling" up will we find a branch
            with a frequency larger thus ours.
            Possible binary search improvement possible?
        */

        node * bubble;
        
        /* Check if are at the end state of tree construction */
        if (tail->parent == &frequencies[0]) {
            bubble = top;

        } else {
            /* Bubble the new branch up */
            bubble = tail;
            while ((bubble->frequency < new_branch->frequency) && bubble != &frequencies[0]) {
                bubble = bubble->parent;
            }
        }

        /* Insert our branch into the double linked list*/
        new_branch->child = bubble->child;
        new_branch->parent = bubble;
        new_branch->parent->child = new_branch;
        new_branch->child->parent = new_branch;
        
        /* Move the tail up to the new smallest node/branch */
        tail = tail->parent->parent;

        /* Remove the smallest node/branch's child as it is now hanging */
        tail->child = NULL;
        
        /* Set the children of the left/right nodes to nothing as they are hanging*/
        new_branch->left->child = NULL;
        new_branch->right->child = NULL;

        /* Set the parents of the left/right nodes to the branch we created for them */
        new_branch->left->parent = new_branch;
        new_branch->right->parent = new_branch;
        
        if (DEBUG) {
            /* Uncomment to iterate through tree construction */ 
            /*
                printf("Total tree:\n");
                print_tree(top, 0);
                printf("Press enter to continue iterating tree...\n");
                int enter = 0;
                while (enter != '\r' && enter != '\n') { enter = getchar(); }
            */
        }
    }

    if (DEBUG) {
        print_tree(top, 0);
        printf("[DEBUG] Tree Done! Congratulations!\n");
    }
}

/**
 * Compress a student integer to a sequence of ones and zeros using
 * a huffman tree.
 * Return void
 * 
 * Inputs
 *  - id                The target student id to convert
 *  - root              Entry into the tree 
 *  - frequencies       Used to find location of leaf in tree 
 *  - num_students      The index of where the leafs end in frequencies
 *  - output            Integer array of ones and zeros 
 *  - top               Size of output
 * Outputs
 *  -  Updated output array to contain compressed student id 
 * 
 */
void id_to_code(int id, node* root, node * frequencies, int num_students, int* output, int *top) {
    
    /* Find the id in the frequencies*/
    node * id_leaf = NULL;
    for (int i=0; i<num_students; i++) {
        if (frequencies[i].id == id) {
            id_leaf = &frequencies[i];
        }
    }

    if (id_leaf == NULL) {
        printf("[ERR] Could not find leaf for %d!\n", id);
        return; 
    }
    
    /*
        Because the tree is doubly linked, we will traverse
        up it to get the compressed output.
        However, we need to remember the previous node
        to "look back" and see if we came from the left
        or right child. 
    */
    
    node * current_leaf = id_leaf;
    node * previous_leaf = id_leaf;

    /* Setup a temporary integer array as we travel left/right */
    int codes [num_students + 2];
    int num_codes=0;
    
    while (current_leaf != root) {
        
        /* Move up the tree */
        current_leaf = current_leaf->parent;
        
        /* Figure out if we moved left or right*/
        if (current_leaf->left == previous_leaf) {
            /* Moved left, add 0 to output*/
            codes[num_codes] = 0;
        } else {
            /* Moved right, add 1 to output */
            codes[num_codes] = 1;
        }

        num_codes = num_codes + 1;
        previous_leaf = current_leaf;
    }

    if (DEBUG) { printf("[DEBUG] Compressing student %d to ", id); }

    /* Reverse order of left/right traversals, save to output */
    for (int i=0; i<num_codes; i++) {
        
        if (codes[num_codes-i -1] == 0) {
            output[*top + i] = 0;
            if (DEBUG) {printf("0");}
        } else {
            output[*top + i] = 1;
            if (DEBUG) {printf("1");}
        }
    }

    if (DEBUG) {printf("\n");}
    
    /*
        This function will be called again with the same output array,
        so we update top to keep track of where we are 
     */
    *top = *top + num_codes;
}

/**
 * Convert an array of ones of zeros and decompress it
 * Return size of decompressed ints
 * 
 * Inputs
 *  - ids             Array of integers to store decompressed results
 *  - codes           Array of ones and zeroes
 *  - num_codes       The number of ones and zeros 
 *  - root            Entry into the huffman tree
 * Outputs
 *  -  Updated ids array to contain decompressed student ids 
 * 
 */
int codes_to_ids(int* ids, int* codes, int num_codes, node* root) {
    
    node* current_node = root;
    int num_ids = 0;
    
    if (DEBUG) {printf("[DEBUG] Decompressing student: ");}

    /* For each code, we traverse down the tree */
    for (int current_code_idx =0; current_code_idx<num_codes; current_code_idx++) {
        if (current_node->is_branch == 1) {
            
            /* 1 for going right, 0 for left*/
            if (codes[current_code_idx] == 1) {
                if (DEBUG) {printf("1");}
                current_node = current_node->right;
            } else {
                if (DEBUG) {printf("0");}
                current_node = current_node->left;
            }

        } else {
            
            /* If we end on a node, then record it's student id */
            if (DEBUG) {printf(" to %d\n", current_node->id);}
            ids[num_ids] = current_node->id;
            num_ids = num_ids + 1;
            current_node = root;
            if (DEBUG) {printf("[DEBUG] Decompressing student: ");}
        
            /* We also go back a code as we read it to see if we decoded a node*/
            current_code_idx = current_code_idx - 1;
            
        }
    }

    /* Flush out the remaining code */
    if (current_node->is_branch == 0) {
        ids[num_ids] = current_node->id;
        num_ids = num_ids + 1;
    }

    if (DEBUG) {printf("\n");}
    return num_ids;
}

/**
 * Convert the huffman tree into a 1d representation using
 * preorder traversal
 * Return void
 * 
 * Inputs
 *  - root             Current node, initialised to tree entry
 *  - serialized       The output array of tree nodes
 *  - top              Current index of serialized tree
 * Outputs
 *  -  updated serialized array to contain tree & top for number of nodes
 * 
 */
void serialize_tree(node* root, int* serialized, int *top) {
    
    /* If it's a leaf, write it's id */
    if (root->is_branch == 0) {
        serialized[*top] = root->id;
        *top = *top + 1;
        return;
    }
    
    /* If it's a branch, write 0 and serialize left/right branches */
    serialized[*top] = 0;
    *top = *top + 1;
    serialize_tree(root->right, serialized, top);
    serialize_tree(root->left, serialized, top);
}

/**
 * "inverse" of serialize_tree, take a 1d representation of a tree
 * and build a tree from it.
 * Return node pointer
 * 
 * Inputs
 *  - serialized    The serialized input tree data
 *  - nodes         Output tree nodes
 *  - top           Current index within nodes, number of nodes
 * Outputs
 *  -  updated nodes array to contain nodes of the tree.
 *     Entry into tree will be the returned node pointer, or first
 *     item in nodes.
 * 
 */
node * deserialize_tree(int* serialized, node* nodes, int *top) {

    /* Check if branch or leaf*/
    if (serialized[*top] == 0) {
        /* Encountered a branch */
        
        /* Define a branch node in nodes */
        int node_index = *top;
        *top = *top + 1;
        nodes[node_index].is_branch = 1;
        nodes[node_index].id = 0;

        /* Root node must have a frequency of 0 for printing */
        if (node_index == 0) {
            nodes[node_index].frequency = 0;
        } else {
            nodes[node_index].frequency = 1;
        }

        /* Deserialize left/right nodes */
        nodes[node_index].right = deserialize_tree(serialized, nodes, top);
        nodes[node_index].left = deserialize_tree(serialized, nodes, top);

        return &nodes[node_index];

    } else {
        /* Encountered a leaf */

        int node_index = *top;

        /* Add leaf node to nodes */
        nodes[node_index].is_branch = 0;
        nodes[node_index].id = serialized[node_index];
        *top = *top + 1;
        return &nodes[node_index];
    }
}