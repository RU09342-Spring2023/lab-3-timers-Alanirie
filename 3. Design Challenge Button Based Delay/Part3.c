/*
 * Part3.c
 *
 *  Created on: Feb 12, 2023
 *      Author: Ayo Overton
 *
 *      YOU NEED TO FILL IN THIS AUTHOR BLOCK
 */

#include <msp430.h>

char TimerState = 0x00; //A starting default timer state

int HoldTime = 0; //The amount of time the user holds down the button is saved in this variable

void gpioInit(); // GPIO initialization
void timerInit(); // Timer initialization

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; //stops the watchdog timer

    gpioInit();
    timerInit();

    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode

    __bis_SR_register(GIE); // Enter LPM3 w/interrupt

    while(1)
    {
        if (TimerState == 0x00)
        {
            //if the timer state is in the initial state
            TB1CTL = TBSSEL_1 | MC_2 | ID_1;
        }
        if (TimerState == 0x01)
        {
            //Button-timed mode
            TB1CTL = TBSSEL_1 | MC_2 | ID_3;
        }
    }
}

void gpioInit()
{
      //Initialize the Red or Green LED
      // Configure RED LED on P1.0 as Output
      P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
      P1DIR |= BIT0;                          // Set P1.0 to output direction

      //Initialize Button 2.3
      // Configure Button on P2.3 as input with pullup resistor
      P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
      P2REN |= BIT3;                          // P2.3 pull-up register enable
      P2IES |= BIT3;                         // P2.3 High --> Low edge, Activates on low
      P2IE |= BIT3;                           // P2.3 interrupt enabled

      //Initialize Button 4.1
      // Configure Button on P4.1 as input with pullup resistor
      P4OUT |= BIT1;                          // Configure P4.1 as pulled-up
      P4REN |= BIT1;                          // P4.1 pull-up register enable
      P4IES |= BIT1;                         // P4.1 High --> Low edge, Activates on low
      P4IE |= BIT1;                           // P4.1 interrupt enabled

}

void timerInit()
{
    // @TODO Initialize Timer B1 in Continuous Mode using ACLK as the source CLK with Interrupts turned on
    TB1CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
    TB1CCR0 = 4096;                           // Used to start CCR
    TB1CTL = TBSSEL_1 | MC_2 | TBCLR | ID_1 | TBIE;                 // ACLK, continuous mode
}

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
    if (TimerState == 0x00)
    {
        P1OUT ^= BIT0;
        TB1CCR0 += 4096; // Add Offset to TB1CCR0
    }
    else if (TimerState == 0x01)
    {
        P1OUT ^= BIT0;
        TB1CCR0 += HoldTime;
    }
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;                         // Clear P1.3 IFG

    TB1CTL = TBSSEL_1 | MC_2 | TBCLR | ID_3 | TBIE;
    TimerState = 0x01;

    while (~P2IN & BIT3)
    {
        HoldTime = TB1R;
    }

}

// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    // @TODO You might need to modify this based on your approach to the lab
    P4IFG &= ~BIT1;                         // Clear P1.3 IFG
    TB1CCR0 = 4096;

    TimerState == 0x00;

}





