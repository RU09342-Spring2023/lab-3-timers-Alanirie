/*
 * Part2.c
 *
 *  Created on: Feb 11, 2023
 *      Author: Ayo Overton
 *
 *      This code will need to change the speed of an LED between 3 different speeds by pressing a button.
 */

#include <msp430.h>

void gpioInit();
void timerInit();

char LEDSpeed = 0x00;

int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    gpioInit();
    timerInit();

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(LPM3_bits | GIE);

    while (1)
    {
        if (LEDSpeed == 0x00)
        {
            TB1CTL = TBSSEL_1 | MC_2 | ID_2;
        }
        else if (LEDSpeed == 0x01)
        {
            TB1CTL = TBSSEL_1 | MC_2 | ID_1;
        }
        else if (LEDSpeed == 0x02)
        {
            TB1CTL = TBSSEL_1 | MC_2 | ID_0;
        }

    }

}


void gpioInit()
{
      //Initialize the Red or Green LED
      // Configure RED LED on P1.0 as Output
      P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
      P1DIR |= BIT0;                          // Set P1.0 to output direction

      // Configure Green LED on P6.6 as Output
      P6OUT &= ~BIT6;                         // Clear P6.6 output latch for a defined power-on state
      P6DIR |= BIT6;                          // Set P6.6 to output direction

      //Initialize Button 2.3
      // Configure Button on P2.3 as input with pullup resistor
      P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
      P2REN |= BIT3;                          // P2.3 pull-up register enable
      P2IES |= BIT3;                         // P2.3 High --> Low edge, Activates on low
      P2IE |= BIT3;                           // P2.3 interrupt enabled

}

void timerInit()
{
    // @TODO Initialize Timer B1 in Continuous Mode using ACLK as the source CLK with Interrupts turned on
    TB1CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
    TB1CCR0 = 32768;                          // Compare the timer value to the number in ccr 0
    TB1CTL = TBSSEL_1 | MC_2 | TBCLR | ID_2 | TBIE;                 // ACLK, continuous mode
}


/*
 * INTERRUPT ROUTINES
 */

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    // @TODO Remember that when you service the GPIO Interrupt, you need to set the interrupt flag to 0.
    // @TODO When the button is pressed, you can change what the CCR0 Register is for the Timer. You will need to track what speed you should be flashing at.

    P2IFG &= ~BIT3;                         // Clear P1.3 IFG
    if ((P2IES &= BIT3) == BIT3)
    {
        switch (LEDSpeed)
        {
            case 0x00:
                LEDSpeed = 0x01;
                break;
            case 0x01:
                LEDSpeed = 0x02;
                break;
            case 0x02:
                LEDSpeed = 0x00;
                break;
        }
    }
    //__bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3

}

//Button interrupt
// Every time you press the button, CCR0 += 1000;
// Timer B1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER1_B0_VECTOR))) Timer1_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    // @TODO You can toggle the LED Pin in this routine and if adjust your count in CCR0.
    P1OUT ^= BIT0;
    TB1CCR0 += 32768;                         // Add Offset to TB1CCR0
}

