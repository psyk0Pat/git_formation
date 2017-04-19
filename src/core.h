/*##########################################################################

 THALES

 COPYRIGHT THALES

 NAE : 07-201
 Maximum level content in the matrix of transfer : 2

 ############################################################################*/
/**
 @application   HLS
 @file          core.h
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

#ifndef __HLS_H_
#define __HLS_H_

#include "display.h"
#include "state_engine.h"
#include "hardware_definition.h"
#include "lsi.h"
#include "log.h"

/* MIN and MAX define */
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define OK 1
#define KO 0

#define NB_MAX_LIBRARIES	100

/**
 * Purpose     : Call a function which parse XML file and handle all memory allocated
 * Input       :
 * Result      : Input file is parsed and stored in a memory
 * Side effect : N/A
 * Re-entrance : N/A
 */
int CORE__dynamic_linking_load_initialization(fptr** tptr_state, unsigned int *ptr_cur_state,
        state_machine_information_table st_info_table, void** tptr_dynamic_lib);

void CORE__drivers_initialization(state_machine_definition** tptr_sm, state_machine_information_table st_info_table);

#endif /* __HLS_H_ */
