
#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void CONTROL_Init(void);
void CONTROL_Loop(void);
void CONTROL_HandleErrors(void);        
void CONTROL_AutonomousDecisions(void); 

#ifdef __cplusplus
}
#endif

#endif /* INC_CONTROL_H_ */
