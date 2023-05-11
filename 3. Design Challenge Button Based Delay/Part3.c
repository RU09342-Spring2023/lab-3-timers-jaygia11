#include <msp430.h>

volatile unsigned int button_down = 0;
volatile unsigned int button_up = 0;
volatile unsigned int timer_value = 0;
const int default_rate = 0x8000;

void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set LED pin as output
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    // Set button pin as input with pull-up resistor
    P4DIR &= ~BIT1;
    P4REN |= BIT1;
    P4OUT |= BIT1;

    // Set reset button pin as input with pull-up resistor
    P2DIR &= ~BIT3;
    P2REN |= BIT3;
    P2OUT |= BIT3;

    // Set Timer B to use ACLK and up mode
    TBCTL = TBSSEL_1 | MC_1;

    // Set Timer B to interrupt on overflow
    TBCCR0 = default_rate;
    TBCTL |= TBIE;

    // Enable global interrupts
    __enable_interrupt();

    // Blink LED at default rate (4Hz)
    while(1)
    {
        // Start timer if not already started
        TBCTL |= MC_1;

        // Toggle LED based on timer state
        if((TBR & 0x8000) == 0)
        {
            P1OUT ^= BIT0;
        }

        __delay_cycles(default_rate);

        // Check if reset button is pressed and reset timer value and blinking rate to default
        if((P2IN & BIT3) == 0)
        {
            timer_value = 0;
            TBCCR0 = default_rate;
        }
    }
}

#pragma vector=TIMERB0_VECTOR
__interrupt void TIMERB0_ISR(void)
{
    // If button is down, increment timer value
    if((P4IN & BIT1) == 0)
    {
        button_down++;
    }
    // If button is up and timer value is non-zero, set timer_value and reset button_down
    else if(button_down)
    {
        button_up++;
        if(button_up == 2) // Add debounce delay
        {
            timer_value = button_down;
            button_down = 0;
            button_up = 0;
            TBCCR0 = timer_value * 32768;
        }
    }
}
