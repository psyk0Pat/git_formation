/*##########################################################################

 THALES

 COPYRIGHT THALES

 NAE : 07-201
 Maximum level content in the matrix of transfer : 2

 ############################################################################*/
/**
 @application   HLS
 @file          display.c
 @author        GAGNAT Pascal
 @date          April 22 2013

 \brief
 Debug function displaying current HLS memory.

 Evolutions  :
 Author      :
 Date        :
 PCR         :
 Description :
 */

/*#########################################################################################################################*/

#include "display.h"

#define __FILE_ID_FOR_LOG__            2

void DISPLAY__state_machine(state_machine_definition* sm)
{
    int i = 0;

    LOG__WRITE_DEBUG("state element:%d\n", sm->state.nb_element);
    for (i = 0; i < sm->state.nb_element; i++)
    {
        LOG__WRITE_DEBUG("	id = %d	name=%s\n", sm->state.table[i].id, sm->state.table[i].name);
    }
    LOG__WRITE_DEBUG("event element:%d\n", sm->event.nb_element);
    for (i = 0; i < sm->event.nb_element; i++)
    {
        LOG__WRITE_DEBUG("	id = %d	name=%s\n", sm->event.table[i].id, sm->event.table[i].name);
    }
    LOG__WRITE_DEBUG("transition:%d\n", sm->nb_transition);
    for (i = 0; i < sm->nb_transition; i++)
    {
        LOG__WRITE_DEBUG("%d	%d	%d\n", sm->transition_table[i].src_state, sm->transition_table[i].ret_event,
                sm->transition_table[i].dst_state);
    }
}

void DISPLAY__hardware_memory(t_plc plc_config)
{
    int i, j, k;

    LOG__WRITE_DEBUG("PLC kind:%s nb_module:%d\n", plc_config.kind, plc_config.nb_module);
    for (i = 0; i < plc_config.nb_module; i++)
    {
        LOG__WRITE_DEBUG("[%d][%d][%d]type:%d	id:%d nb_board:%d\n", i, 0, 0, plc_config.ptr_module[i].type,
                plc_config.ptr_module[i].id, plc_config.ptr_module[i].nb_board);
        for (j = 0; j < plc_config.ptr_module[i].nb_board; j++)
        {
            LOG__WRITE_DEBUG("[%d][%d][%d]name:%s nb_signal:%d\n", i, j, 0, plc_config.ptr_module[i].ptr_board[j].name,
                    plc_config.ptr_module[i].ptr_board[j].nb_signal);
            for (k = 0; k <= plc_config.ptr_module[i].ptr_board[j].nb_signal; k++)
            {
                LOG__WRITE_DEBUG("[%d][%d][%d]id:%d tag:%s value:%d\n", i, j, k,
                        plc_config.ptr_module[i].ptr_board[j].ptr_signal[k].id,
                        plc_config.ptr_module[i].ptr_board[j].ptr_signal[k].tag,
                        plc_config.ptr_module[i].ptr_board[j].ptr_signal[k].value);
            }
        }
    }
}

