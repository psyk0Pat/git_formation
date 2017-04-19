#include "core.h"
#include <dlfcn.h>

#define __FILE_ID_FOR_LOG__            0

int CORE__dynamic_linking_load_initialization(fptr** tptr_state, unsigned int *ptr_cur_state,
        state_machine_information_table st_info_table, void** tptr_dynamic_lib)
{

    unsigned int i;
    fptr (*tptr_function_lib)(fptr* table);

    if (st_info_table.number > NB_MAX_LIBRARIES)
    {
        printf("Maximum number of libraries to be loaded reached(>%d)\n",
        NB_MAX_LIBRARIES);
        return KO;
    }

    /** Dynamic linking loader initialization */
    for (i = 0; i < st_info_table.number; i++)
    {
        st_info_table.st_info[i].thread_status = E_FALSE;
        tptr_dynamic_lib[i] = dlopen(st_info_table.st_info[i].libname,
        RTLD_NOW);
        if (tptr_dynamic_lib[i] == NULL)
        {
            printf("Library %s not found: %s\n", st_info_table.st_info[i].id, dlerror());
            tptr_state[i] = NULL;
        }
        else
        {
            tptr_function_lib = dlsym(tptr_dynamic_lib[i], "get_sm_function");
            if (!tptr_function_lib)
            {
                printf("Problem calling get_sm_function() from %s: %s\n", st_info_table.st_info[i].id, dlerror());
                tptr_state[i] = NULL;
            }
            else
            {
                printf("Library %s loaded\n", st_info_table.st_info[i].id);
                tptr_state[i] = (fptr*) tptr_function_lib(tptr_state[i]);
                ptr_cur_state[i] = 0;
            }
        }
    }
    return OK;
}

void CORE__drivers_initialization(state_machine_definition** tptr_sm, state_machine_information_table st_info_table)
{

    xmlDocPtr ptr_doc = NULL;
    xmlNodePtr ptr_cur = NULL;
    unsigned int* ptr_transition_line;
    unsigned int i;

    /** Drivers initialization */
    for (i = 0; i < st_info_table.number; i++)
    {
        tptr_sm[i] = (state_machine_definition*) malloc(sizeof(state_machine_definition));
        tptr_sm[i]->nb_transition = 0;
        ptr_transition_line = (unsigned int*) malloc(sizeof(unsigned int));
        STATE_ENGINE__initializeParsing(ptr_doc, ptr_cur, st_info_table.st_info[i].filename, "scxml");
        STATE_ENGINE__initializeStateMachine(ptr_doc, ptr_cur, tptr_sm[i], 0);
        tptr_sm[i]->transition_table = (struct transition *) malloc(
                tptr_sm[i]->nb_transition * sizeof(struct transition));
        ptr_cur = xmlDocGetRootElement(ptr_doc);
        *ptr_transition_line = 0;

        /** parsing data */
        STATE_ENGINE__parseBody(ptr_doc, ptr_cur, tptr_sm[i], 0, ptr_transition_line);
        STATE_ENGINE__deleteParsing(ptr_doc);

        free(ptr_transition_line);
        free(ptr_cur);
        free(ptr_doc);
    }
}

/** Entry point for processing */
int main()
{
    int i = 0;
    state_machine_definition** tptr_sm = NULL;
    fptr** tptr_state = NULL;
    unsigned int *ptr_cur_state = NULL;
    state_machine_information_table st_info_table;
    void** tptr_dynamic_lib = NULL;
    state_engine_memory state_engine_cycle_s;
    void* ptr_thread_return = NULL;
    char* ptr_name = NULL;
    xmlDocPtr ptr_doc = NULL;
    xmlNodePtr ptr_cur = NULL;
    unsigned int log_level;
    unsigned int log_mode;
//	t_plc* plc_config;

//TODO : get plc_config from ms_daemon

    log_mode = LOG__LOCAL;
    log_level = LOG__DEBUG;
    log__initialize(1, log_level, log_mode);

    LOG__WRITE_INFO("HLS Starting...\n");

    LSI__lsi_initialisation();

    while (1)
        ;

//	xmlInitParser();
//	plc_config = (t_plc*) malloc(sizeof(t_plc));
//	HARDWARE_DEFINITION__parsing(plc_config);

    ptr_name = (char*) malloc(50 * sizeof(char));
    snprintf(ptr_name, 25, "./state_scheduling.xml");

    /** Main configuration file initialization */
    STATE_ENGINE__initializeParsing(ptr_doc, ptr_cur, ptr_name, "xml");

    st_info_table.number = STATE_ENGINE__child_node_count(ptr_cur, "sm");
    if (st_info_table.number == 0)
    {
        LOG__WRITE_CRIT("file %s empty or corrupted (1)\n", ptr_name);
        xmlFree(ptr_cur);
        xmlFreeDoc(ptr_doc);
        free(ptr_name);
        return KO;
    }

    /** Memory allocation */
    tptr_sm = (state_machine_definition**) malloc(st_info_table.number * sizeof(state_machine_definition*));
    tptr_state = (fptr**) malloc(st_info_table.number * sizeof(fptr*));
    ptr_cur_state = (unsigned int*) malloc(st_info_table.number * sizeof(unsigned int));
    st_info_table.st_info = (state_machine_information*) malloc(
            st_info_table.number * sizeof(state_machine_information));
    tptr_dynamic_lib = (void**) malloc(st_info_table.number * sizeof(void*));

    /** Parse main configuration file */
    if (STATE_ENGINE__parseMainBody(ptr_doc, ptr_cur, st_info_table.st_info) == 0)
    {
        LOG__WRITE_CRIT("file %s empty or corrupted (2)\n", ptr_name);

        free(st_info_table.st_info);
        xmlFree(ptr_cur);
        xmlFreeDoc(ptr_doc);
        free(ptr_name);
        return KO;
    }

    /* No need to check XML file anymore starting this point */
    xmlFree(ptr_cur);
    xmlFreeDoc(ptr_doc);

    if (CORE__dynamic_linking_load_initialization(tptr_state, ptr_cur_state, st_info_table, tptr_dynamic_lib) == 0)
    {

        free(ptr_name);
        return KO;
    }
    CORE__drivers_initialization(tptr_sm, st_info_table);

    /** DEBUG : Display memory */
    for (i = 0; i < 2; i++)
    {
//		DISPLAY__state_machine(tptr_sm[i]);
    }

    /** State engine */
    state_engine_cycle_s.cur_state = ptr_cur_state;
    state_engine_cycle_s.state = tptr_state;
    state_engine_cycle_s.sm = tptr_sm;
    state_engine_cycle_s.st_info_table = st_info_table;
    pthread_mutex_init(&m_thread_status, NULL);
    STATE_ENGINE__cycle(&state_engine_cycle_s);

    /** Join threads if existing */
    for (i = 0; i < st_info_table.number; i++)
    {
        if (state_engine_cycle_s.state[i] != NULL
                && state_engine_cycle_s.st_info_table.st_info[i].scheduling_mode
                        == INDEPENDANT&& &state_engine_cycle_s.st_info_table.st_info[i].scheduling_thread
                        != NULL)
        {
            (void) pthread_join(state_engine_cycle_s.st_info_table.st_info[i].scheduling_thread, &ptr_thread_return);
        }
    }

    /** Dynamic linking loader free */
    for (i = 0; i < st_info_table.number; i++)
    {
        if (tptr_dynamic_lib[i] != NULL)
        {
            dlclose(tptr_dynamic_lib[i]);
        }
    }

    free(ptr_name);
    free(tptr_sm);
    pthread_exit(0);

    while (1)
        ;

    return KO;
}
