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

/* Using this struct the whole arm can be described in terms of where the servos are */
typedef struct {
	uint16_t usBaseRotation, 
			 usBaseAngle,
			 usElbowAngle,
			 usWristPitch,
			 usWristRoll,
			 usGrabber;
} arm_position_t
    
// Leave the original functions in for testing
uint16_t usGetCurrentPosition( void );
void    vSetCurrentPosition( uint16_t usNewPosition );

void vSetCurrentArmPosition( arm_position_t xNewPosition );
arm_position_t xGetCurrentPosition( void );
    
#endif

/* [] END OF FILE */
