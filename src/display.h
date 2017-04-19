/*##########################################################################

 THALES

 COPYRIGHT THALES

 NAE : 07-201
 Maximum level content in the matrix of transfer : 2

 ############################################################################*/
/**
 @application   HLS
 @file          display.h
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

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <stdio.h>

#include "state_engine.h"
#include "hardware_definition.h"
#include "log.h"

void DISPLAY__state_machine(state_machine_definition* sm);
void DISPLAY__hardware_memory(t_plc plc_config);

#endif /* __DISPLAY_H_ */
