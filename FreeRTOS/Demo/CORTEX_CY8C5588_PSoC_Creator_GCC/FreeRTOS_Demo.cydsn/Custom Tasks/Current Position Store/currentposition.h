/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef CURRENTPOSITION_H
#define CURRENTPOSITION_H

#include <stdint.h>
    
/* Using this struct the whole arm can be described in terms of where the servos are */
typedef struct {
	uint16_t usBaseRotation, 
			 usBaseElevation,
			 usElbow,
			 usWristPitch,
			 usWristRoll,
			 usGrabber;
}  xArmPosition_t;
    
// Leave the original functions in for testing
uint16_t usGetCurrentPosition( void );
void    vSetCurrentPosition( uint16_t usNewPosition );

void vSetCurrentArmPosition(  xArmPosition_t xNewPosition );
 xArmPosition_t xGetCurrentPosition( void );
    
#endif

/* [] END OF FILE */
