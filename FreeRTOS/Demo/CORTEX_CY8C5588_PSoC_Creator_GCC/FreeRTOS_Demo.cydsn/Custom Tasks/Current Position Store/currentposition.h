/************ CHANGE LOG ****************
Change ID      : NA
Version        : 1
Date           : 3rd Jan 2017
Changes Made   : Initial Issue
*****************************************/

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
    
void vSetCurrentArmPosition(  xArmPosition_t xNewPosition );
 xArmPosition_t xGetCurrentPosition( void );
    
#endif

/* [] END OF FILE */
