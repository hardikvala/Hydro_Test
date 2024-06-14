/*
 * gsm.h
 *
 *  Created on: Jun 14, 2024
 *      Author: hardi
 */

#ifndef INC_GSM_H_
#define INC_GSM_H_

#include "main.h"
#include <string.h>
#include <stdio.h>

// Function prototypes
void GSM_Init(void);
void GSM_SendCommand(const char *command);
void GSM_ReadResponse(void);
void GSM_CleanResponse(uint8_t *response);


#endif /* INC_GSM_H_ */
