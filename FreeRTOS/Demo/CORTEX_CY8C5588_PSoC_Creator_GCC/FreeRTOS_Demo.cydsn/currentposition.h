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

typedef struct {
	uint16_t usBaseRotation, 
			 usBaseAngle,
			 usElbowAngle,
			 usWristPitch,
			 usWristRoll,
			 usGrabber;
} arm_position_t
    
uint16_t usGetCurrentPosition( void );
void    vSetCurrentPosition( uint16_t usNewPosition );
    
#endif

/* [] END OF FILE */
