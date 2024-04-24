#include <msp430.h>

#define Motor 0
#define rpm 1

// 0 1.4
// 1 1.3

unsigned char mensaje1[]={"### \r\n"};
unsigned  int ADC_samples[2];
unsigned long j;

unsigned char i;
unsigned char dato;
#define GREEN   BIT6                // Green LED -> P1.6
#define AIN     BIT3

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    TA0CCR0 += 10000;

    ADC10CTL0 &= ~ENC;
    ADC10SA = (unsigned int) & ADC_samples;
    ADC10CTL0 |= ENC + ADC10SC;
}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void){
        //ADC 1 input
        mensaje1[ 2 ] = (ADC_samples[rpm] % 10) + '0';
        ADC_samples[rpm] /= 10;
        mensaje1[ 1 ] = (ADC_samples[rpm] % 10) + '0';
        ADC_samples[rpm] /= 10;
        mensaje1[0] = (ADC_samples[rpm] % 10) + '0';
        ADC_samples[rpm] /= 10;

        TACCR1 = ADC_samples[Motor]*60;                        // Set Duty Cycle = ADC Value

        IE2 |= UCA0TXIE;
}



#pragma vector=USCIAB0TX_VECTOR       // A partir línea 940   ISR:Interrupt Service Routine
__interrupt void UART0_ISR (void)   // Includes>C:/ti/ccs1230/ccs_base...                           //    ->msp430g2553.h
{
    if(mensaje1[i] != '\0'){
        while(! (IFG2 & UCA0TXIFG) );
        UCA0TXBUF = mensaje1[i++];
        for(j=0; j < 1000; j++);
    } else {
        IE2 &= ~UCA0TXIE;
        i=0;
    }
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    // Timer
    TA0CTL = TASSEL_2 + MC_1;
    TA0CCTL0 = CCIE;
    //TA0CCR0 = TAR + 10000;
    TA0CCR0 = 10000;
    // adc
    ADC10AE0 |= AIN + BIT4;                     // P1.3 ADC option select
    ADC10DTC1 = 2;
    ADC10CTL1 = INCH_4 + CONSEQ_1;                         // ADC Channel -> 1 (P1.3)
    ADC10CTL0 = SREF_0 + ADC10SHT_2 + MSC + ADC10IE +ADC10ON;  // Ref -> Vcc, 64 CLK S&H

    // pwm
    P1DIR |= GREEN;                             // Green LED -> Output
    P1SEL |= GREEN;                             // Green LED -> Select Timer Output
    //CCR0 = 1023;                                // Set Timer0 PWM Period
    CCTL1 = OUTMOD_7;                           // Set TA0.1 Waveform Mode
    TACCR1 = 0;                                   // Set TA0.1 PWM duty cycle
    //TACTL = TASSEL_2 + MC_1;                    // Timer Clock -> SMCLK, Mode -> Up Count

    // uart
    P1SEL |=BIT2+BIT1;        //Configura pines como UART
    P1SEL2 |=BIT2+BIT1;
    UCA0CTL1=BIT7;            //Selecciona SMCLK como reloj de UART (1 MHz)
    UCA0BR0=104;

    while(1)
    {
        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

        while(ADC10CTL1 & ADC10BUSY);           // Wait for conversion to end

        //TACCR1 = ADC_samples[Motor];                        // Set Duty Cycle = ADC Value

        IE2=BIT1;               //Hab interrupcion Tx

        __bis_SR_register(GIE);    // Global Enable interrupt

    }
}
