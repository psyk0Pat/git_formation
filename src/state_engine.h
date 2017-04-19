/*##########################################################################

 THALES

 COPYRIGHT THALES

 NAE : 07-201
 Maximum level content in the matrix of transfer : 2

 ############################################################################*/
/**
 @application   HLS
 @file          state_engine.h
 @author        GAGNAT Pascal
 @date          April 22 2013

 \brief
 Structures, variables and functions definitions

 Evolutions  :
 Author      :
 Date        :
 PCR         :
 Description :
 */

/*#########################################################################################################################*/

#ifndef __STATE_ENGINE_H_
#define __STATE_ENGINE_H_

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <log.h>

/** XML specifics headers */
#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpathInternals.h>

/** State machine XML file */
#define MAX_STATE_NAME 			50
#define MAX_EVENT_NAME 			50

/** Scheduling XML file */
#define MAX_SCHEDULING_PATH 		200
#define MAX_SCHEDULING_ID			50
#define MAX_SCHEDULING_DEFINITION 	100
#define MAX_SCHEDULING_LIBRARY		300

typedef enum
{
    E_FALSE = (0), /**  FALSE */
    E_TRUE = (1), /**  TRUE  */
} E_BOOLEAN;

typedef enum
{
    INDEPENDANT = (0), /**  INDEPENDANT */
    SEQUENTIAL = (1), /**  SEQUENTIAL  */
} E_SCHEDULING_MODE;

typedef struct
{
    char* name; /** name of element   */
    unsigned int id; /** element ID        */
} element_table;

/** can be event or state */
struct element
{
    element_table* table; /** table of elements */
    int nb_element; /** number of element */
};

struct transition
{
    unsigned int src_state; /** source state      */
    unsigned int ret_event; /** event             */
    unsigned int dst_state; /** destination state */
};

typedef struct
{
    struct element state; /** table of states                      */
    struct element event; /** table of events                      */
    struct transition *transition_table; /** table containing all the transitions */
    unsigned int nb_transition; /** number of transition                 */
    unsigned int initial_state; /** initial state                        */
} state_machine_definition;

typedef struct
{
    char* id; /** state machine ID                      */
    char* filename; /** define state machine file name        */
    char* libname; /** define state machine library name     */
    unsigned int priority; /** define in which order state machine is executed. 0 means independent execution */
    E_SCHEDULING_MODE scheduling_mode; /** define scheduling mode                */
    pthread_t scheduling_thread; /** define thread process if priority is set to 0 */
    E_BOOLEAN thread_status; /** define the status of thread creation. E_FALSE=under creation, E_TRUE=created */
} state_machine_information;

typedef struct
{
    state_machine_information* st_info; /** state machine information table       */
    unsigned int number; /** number of state machine information   */
} state_machine_information_table;

typedef int (*fptr)(void); /** function pointer                      */

typedef struct
{
    unsigned int* cur_state;
    fptr** state;
    state_machine_definition** sm;
    state_machine_information_table st_info_table;
} state_engine_memory;

unsigned int STATE_ENGINE__lookup_transitions(unsigned int cur_state, unsigned int code_return,
        state_machine_definition* sm);
unsigned int STATE_ENGINE__cycle(state_engine_memory* state_engine_cycle_s);
unsigned int STATE_ENGINE__cycle_thread(state_engine_memory* state_engine_cycle_s);

unsigned int STATE_ENGINE__child_node_count(const xmlNodePtr nodePtr, char* name);
void STATE_ENGINE__initializeStateMachine(xmlDocPtr document, xmlNodePtr current, state_machine_definition* sm,
        unsigned int pos);
void STATE_ENGINE__parseBody(xmlDocPtr document, xmlNodePtr current, state_machine_definition* sm, unsigned int index,
        unsigned int* transition_line);
unsigned int STATE_ENGINE__parseMainBody(xmlDocPtr document, xmlNodePtr current, state_machine_information* st_info);
void STATE_ENGINE__initializeParsing(xmlDocPtr doc, xmlNodePtr cur, char* name, char* tag);
void STATE_ENGINE__deleteParsing(xmlDoc* doc);

pthread_mutex_t m_thread_status;

#endif /* __STATE_ENGINE_H_ */
