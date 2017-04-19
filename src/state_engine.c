/*##########################################################################

 THALES

 COPYRIGHT THALES

 NAE : 07-201
 Maximum level content in the matrix of transfer : 2

 ############################################################################*/
/**
 @application   HLS
 @file          state_engine.c
 @author        GAGNAT Pascal
 @date          April 22 2013

 \brief
 State engine.
 Change current state of a specific state machine according to its specific transition table.

 Evolutions  :
 Author      :
 Date        :
 PCR         :
 Description :
 */

/*#########################################################################################################################*/

#include "state_engine.h"

#define __FILE_ID_FOR_LOG__            3

unsigned int STATE_ENGINE__lookup_transitions(unsigned int cur_state, unsigned int code_return,
        state_machine_definition* ptr_sm)
{
    int new_state = 0;
    unsigned int ref_src_state, id_src_state;
    unsigned int ref_dst_state, id_dst_state;
    unsigned int ref_ret_event, id_ret_event;
    unsigned int i, j;
    unsigned int is_rule_found = 0;

    id_src_state = 0;
    id_dst_state = 0;
    id_ret_event = 0;
    for (i = 0; i < ptr_sm->nb_transition; i++)
    {
        if (ptr_sm->transition_table[i].src_state == cur_state && ptr_sm->transition_table[i].ret_event == code_return)
        {
            ref_src_state = ptr_sm->transition_table[i].src_state;
            for (j = 0; j < ptr_sm->state.nb_element; j++)
            {
                if (ptr_sm->state.table[j].id == ref_src_state)
                {
                    id_src_state = j;
                }
            }
            ref_dst_state = ptr_sm->transition_table[i].dst_state;
            for (j = 0; j < ptr_sm->state.nb_element; j++)
            {
                if (ptr_sm->state.table[j].id == ref_dst_state)
                {
                    id_dst_state = j;
                }
            }
            ref_ret_event = ptr_sm->transition_table[i].ret_event;
            for (j = 0; j < ptr_sm->event.nb_element; j++)
            {
                if (ptr_sm->event.table[j].id == ref_ret_event)
                {
                    id_ret_event = j;
                }
            }
            printf("	%s(%d)	--%s(%d)-->	%s(%d)\n", ptr_sm->state.table[id_src_state].name, ref_src_state,
                    ptr_sm->event.table[id_ret_event].name, ref_ret_event, ptr_sm->state.table[id_dst_state].name,
                    ref_dst_state);
            new_state = ptr_sm->transition_table[i].dst_state;
            is_rule_found = 1;
        }
    }

    if (is_rule_found == 0)
    {
        LOG__WRITE_WARN("No rules found for this transition: %d --%d--> ?\n", cur_state, code_return);
        return cur_state;
    }
    else
    {
        return new_state;
    }
}

unsigned int STATE_ENGINE__cycle(state_engine_memory* ptr_state_engine_cycle_s)
{
    int (*state_functions)(void);
    unsigned int ret_event = 0;
    unsigned int j = 0;
    unsigned int i = 0;

    printf("\n*** cycle 1***\n");
    for (;;)
    {
        if (ptr_state_engine_cycle_s->state[i] != NULL
                && ptr_state_engine_cycle_s->st_info_table.st_info[i].scheduling_mode == INDEPENDANT&&
                &ptr_state_engine_cycle_s->st_info_table.st_info[i].scheduling_thread != NULL)
        {

            pthread_mutex_lock(&m_thread_status);
            if (ptr_state_engine_cycle_s->st_info_table.st_info[i].thread_status == E_FALSE)
            {
                printf("Thread %s started\n", ptr_state_engine_cycle_s->st_info_table.st_info[i].id);
                pthread_create(&ptr_state_engine_cycle_s->st_info_table.st_info[i].scheduling_thread,
                NULL, (void*) STATE_ENGINE__cycle_thread, ptr_state_engine_cycle_s);
            }
            else
            {
                pthread_mutex_unlock(&m_thread_status);
            }

        }
        else if (ptr_state_engine_cycle_s->state[i] != NULL
                && ptr_state_engine_cycle_s->st_info_table.st_info[i].scheduling_mode == SEQUENTIAL)
        {

            state_functions = ptr_state_engine_cycle_s->state[i][ptr_state_engine_cycle_s->cur_state[i]];
            ret_event = state_functions();
            ptr_state_engine_cycle_s->cur_state[i] = STATE_ENGINE__lookup_transitions(
                    ptr_state_engine_cycle_s->cur_state[i], ret_event, ptr_state_engine_cycle_s->sm[i]);
        }
        i++;
        if (i == ptr_state_engine_cycle_s->st_info_table.number)
        {
            j++;
            i = 0;
            printf("\n*** cycle %d***\n", j + 1);

        }
        if (j == 10)
            return 0;
    }
    return 0;
}

unsigned int STATE_ENGINE__cycle_thread(state_engine_memory* ptr_state_engine_cycle_s)
{
    int (*state_functions)(void);
    unsigned int ret_event = 0;
    unsigned int i = 0;
    unsigned int j = 0;

    for (i = 0; i < ptr_state_engine_cycle_s->st_info_table.number; i++)
    {
        if (ptr_state_engine_cycle_s->state[i] != NULL
                && ptr_state_engine_cycle_s->st_info_table.st_info[i].scheduling_mode == INDEPENDANT
                && &ptr_state_engine_cycle_s->st_info_table.st_info[i].scheduling_thread != NULL
                && ptr_state_engine_cycle_s->st_info_table.st_info[i].thread_status == E_FALSE)
        {

            ptr_state_engine_cycle_s->st_info_table.st_info[i].thread_status = E_TRUE;
            pthread_mutex_unlock(&m_thread_status);
            printf("\n*** THREAD CYCLE 1***\n");
            for (;;)
            {
                state_functions = ptr_state_engine_cycle_s->state[i][ptr_state_engine_cycle_s->cur_state[i]];
                ret_event = state_functions();
                ptr_state_engine_cycle_s->cur_state[i] = STATE_ENGINE__lookup_transitions(
                        ptr_state_engine_cycle_s->cur_state[i], ret_event, ptr_state_engine_cycle_s->sm[i]);
                i++;
                if (i == ptr_state_engine_cycle_s->st_info_table.number)
                {
                    j++;
                    i = 0;
                    printf("\n*** THREAD CYCLE %d***\n", j + 1);

                }
                if (j == 10)
                {
                    pthread_exit(NULL);
                    return 0;
                }
            }
        }
    }
    pthread_mutex_unlock(&m_thread_status);
    pthread_exit(NULL);
    return 0;
}

void STATE_ENGINE__parseBody(xmlDocPtr ptr_document, xmlNodePtr ptr_current, state_machine_definition* ptr_sm,
        unsigned int index, unsigned int* ptr_transition_line)
{

    xmlChar *ptr_uri;
    xmlDocPtr ptr_doc = ptr_document;
    xmlNodePtr ptr_cur = ptr_current;
    unsigned int i = 0;
    unsigned int state_index = 0;

    ptr_cur = ptr_cur->xmlChildrenNode;

    while (ptr_cur != NULL)
    {

        // Parsing final state
        if (!xmlStrcmp(ptr_cur->name, (const xmlChar *) "final") || !xmlStrcmp(ptr_cur->name, (const xmlChar *) "state")
                || !xmlStrcmp(ptr_cur->name, (const xmlChar *) "initial"))
        {

            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "id");

            for (i = 0; i < ptr_sm->state.nb_element; i++)
            {
                if (!strcmp(ptr_sm->state.table[i].name, (const char*) ptr_uri))
                {
                    state_index = ptr_sm->state.table[i].id;
                }
            }
            xmlFree(ptr_uri);
            STATE_ENGINE__parseBody(ptr_doc, ptr_cur, ptr_sm, state_index, ptr_transition_line);
        }

        // Parsing general state
        else if (!xmlStrcmp(ptr_cur->name, (const xmlChar *) "transition"))
        {
            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "target");
            for (i = 0; i < ptr_sm->state.nb_element; i++)
            {
                if (!strcmp(ptr_sm->state.table[i].name, (const char*) ptr_uri))
                {
                    state_index = ptr_sm->state.table[i].id;
                }
            }
            xmlFree(ptr_uri);

            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "event");
            for (i = 0; i < ptr_sm->event.nb_element; i++)
            {
                if (!strcmp(ptr_sm->event.table[i].name, (const char*) ptr_uri))
                {
                    ptr_sm->transition_table[*ptr_transition_line].src_state = index;
                    ptr_sm->transition_table[*ptr_transition_line].ret_event = ptr_sm->event.table[i].id;
                    ptr_sm->transition_table[*ptr_transition_line].dst_state = state_index;
                    *ptr_transition_line = *ptr_transition_line + 1;
                }
            }
            xmlFree(ptr_uri);
        }
        ptr_cur = ptr_cur->next;
    }
}

void STATE_ENGINE__initializeStateMachine(xmlDocPtr ptr_document, xmlNodePtr ptr_current,
        state_machine_definition* ptr_sm, unsigned int pos)
{

    xmlChar *ptr_uri;
    xmlDocPtr ptr_doc = ptr_document;
    xmlNodePtr ptr_cur = ptr_current;
    int i = 0;
    unsigned int nb_element;

    ptr_cur = ptr_cur->xmlChildrenNode;

    while (ptr_cur != NULL)
    {
        if (!xmlStrcmp(ptr_cur->name, (const xmlChar *) "datamodel"))
        {
            nb_element = STATE_ENGINE__child_node_count(ptr_cur, "data");
            if (i == 0)
            {
                ptr_sm->state.nb_element = nb_element;
                ptr_sm->state.table = (element_table*) malloc(ptr_sm->state.nb_element * sizeof(element_table));
            }
            else
            {
                ptr_sm->event.nb_element = nb_element;
                ptr_sm->event.table = (element_table*) malloc(ptr_sm->event.nb_element * sizeof(element_table));
            }
            STATE_ENGINE__initializeStateMachine(ptr_doc, ptr_cur, ptr_sm, i);
            i++;
        }
        else if (!xmlStrcmp(ptr_cur->name, (const xmlChar *) "data"))
        {
            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "src");
            if (pos == 0)
            {
                ptr_sm->state.table[i].id = atoi((const char*) ptr_uri);
            }
            else
            {
                ptr_sm->event.table[i].id = atoi((const char*) ptr_uri);
            }
            xmlFree(ptr_uri);

            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "id");

            /** Retrieving state name data model */
            if (pos == 0)
            {
                ptr_sm->state.table[i].name = (char*) malloc((strlen((const char*) ptr_uri) + 1) * sizeof(char));
                snprintf(ptr_sm->state.table[i].name, MAX_STATE_NAME, "%s", (const char*) ptr_uri);
            }

            /** Retrieving event name data model */
            else
            {
                ptr_sm->event.table[i].name = (char*) malloc((strlen((const char*) ptr_uri) + 1) * sizeof(char));
                snprintf(ptr_sm->event.table[i].name, MAX_EVENT_NAME, "%s", (const char*) ptr_uri);
            }
            xmlFree(ptr_uri);

            i++;
        }
        if (!xmlStrcmp(ptr_cur->name, (const xmlChar *) "final") || !xmlStrcmp(ptr_cur->name, (const xmlChar *) "state")
                || !xmlStrcmp(ptr_cur->name, (const xmlChar *) "initial"))
        {
            ptr_sm->nb_transition = ptr_sm->nb_transition + STATE_ENGINE__child_node_count(ptr_cur, "transition");
        }
        ptr_cur = ptr_cur->next;
    }
}

/** Parsing state scheduling XML file */
unsigned int STATE_ENGINE__parseMainBody(xmlDocPtr ptr_document, xmlNodePtr ptr_current,
        state_machine_information* ptr_st_info)
{
    xmlChar* ptr_uri;
    xmlNodePtr ptr_cur = ptr_current;
    unsigned int i = 0;
    char* ptr_path;

    ptr_cur = ptr_cur->xmlChildrenNode;

    while (ptr_cur != NULL)
    {

        // Parsing state machine
        if (!xmlStrcmp(ptr_cur->name, (const xmlChar *) "sm"))
        {

            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "path");
            ptr_path = (char*) malloc((strlen((const char*) ptr_uri) + 1) * sizeof(char));
            snprintf(ptr_path, MAX_SCHEDULING_PATH, "%s", (const char*) ptr_uri);

            xmlFree(ptr_uri);

            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "id");
            ptr_st_info[i].id = (char*) malloc((strlen((const char*) ptr_uri) + 1) * sizeof(char));
            snprintf(ptr_st_info[i].id, MAX_SCHEDULING_ID, "%s", (const char*) ptr_uri);
            xmlFree(ptr_uri);

            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "definition");
            ptr_st_info[i].filename = (char*) malloc(
                    (strlen((const char*) ptr_uri) + strlen(ptr_path) + 1) * sizeof(char));
            snprintf(ptr_st_info[i].filename, MAX_SCHEDULING_DEFINITION, "%s%s", ptr_path, ptr_uri);
            xmlFree(ptr_uri);

            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "scheduling");
            ptr_st_info[i].priority = atoi((const char*) ptr_uri);
            xmlFree(ptr_uri);
            if (ptr_st_info[i].priority != 0)
            {
                ptr_st_info[i].scheduling_mode = SEQUENTIAL;
            }
            else
            {
                ptr_st_info[i].scheduling_mode = INDEPENDANT;

            }

            ptr_uri = xmlGetProp(ptr_cur, (const xmlChar *) "library");
            ptr_st_info[i].libname = (char*) malloc(
                    (strlen((const char*) ptr_uri) + strlen(ptr_path) + 1) * sizeof(char));
            snprintf(ptr_st_info[i].libname, MAX_SCHEDULING_LIBRARY, "%s%s", ptr_path, ptr_uri);
            xmlFree(ptr_uri);

            free(ptr_path);
            i++;
        }
        ptr_cur = ptr_cur->next;
    }
    return i;
}

unsigned int STATE_ENGINE__child_node_count(const xmlNodePtr nodePtr, char* ptr_name)
{

    xmlNodePtr node;
    register int i = 0;

    for (node = nodePtr->children; node != NULL; node = node->next)
    {
        if (node != NULL && !xmlStrcmp(node->name, (const xmlChar *) ptr_name))
        {
            i++;
        }
    }
    if (ptr_name == NULL)
    {
        return i / 2;
    }
    else
    {
        return i;
    }
}

void STATE_ENGINE__initializeParsing(xmlDocPtr ptr_doc, xmlNodePtr ptr_cur, char* ptr_name, char* ptr_tag)
{
    ptr_doc = xmlParseFile(ptr_name);

    if (ptr_doc == NULL)
    {
        LOG__WRITE_CRIT("file %s not parsed successfully\n", ptr_name);
        exit(1);
    }

    ptr_cur = xmlDocGetRootElement(ptr_doc);
    if (ptr_cur == NULL)
    {
        xmlFreeDoc(ptr_doc);
        LOG__WRITE_CRIT("file %s corrupted (1)\n", ptr_name);
        exit(1);
    }

    if (xmlStrcmp((ptr_cur)->name, (const xmlChar*) ptr_tag))
    {
        xmlFreeDoc(ptr_doc);
        LOG__WRITE_CRIT("file %s corrupted (2)\n", ptr_name);
        exit(1);
    }
}

void STATE_ENGINE__deleteParsing(xmlDocPtr ptr_doc)
{
    xmlFreeDoc(ptr_doc);
    xmlCleanupParser();
}
