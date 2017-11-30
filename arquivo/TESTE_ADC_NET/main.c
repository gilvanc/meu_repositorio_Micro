#include <msp430g2553.h>

// Global variables

int j = 0;
int adc[64] = {0}; //Sets up an array of 10 integers and zero's the values
volatile unsigned int enable=0, T0=0, T1=0,tempo = 0;
volatile unsigned int Temp_exec1 = 0;//,Temp_exec2 = 0, Temp_exec3 = 0;
int bpm = 0;
char word[10] = {'C','O','M','P','L','E','T','E','D',' '};

// Function prototypes

void adc_Setup();
void adc_Sam10();
void TimerA_configure();
void setUART();
void Transmitir(int tempo, char sinal);

void main()
{
      unsigned int i=0;
      WDTCTL = WDTPW + WDTHOLD;         // Stop WDT
      BCSCTL1 |= XTS + DIVA_3; //32768
      BCSCTL2 |= DIVA_3; //32768/8 =  4,096kHz
      BCSCTL3 &= ~(LFXT1S0 + LFXT1S1);
      DCOCTL = CALDCO_1MHZ;
      adc_Setup();// Fucntion call for adc_setup
      setUART();
      while(1)
      {
          adc_Sam10();      // Function call for adc_samp
          TimerA_configure();
          enable = 0;
          for (i=1; i<65 ; i++){
              if (enable ==2)
                  break;
              else if(adc[i-1]<30 & adc[i]<800){
                  enable++;
                  TA0CCTL0 ^= CCIS0;  // Interrupt request (capture condition)
              }
          }
          bpm = (60000/Temp_exec1);
          int tens = (bpm / 10) + 48;
          int ones = (bpm % 10) + 48;
          word[0] = tens;
          word[1] = ones;
          while(1){


             Transmitir(0, word[j]); //RECEPTOR PUTTY COM5
             j++;

             if(j == 11){
               break;
            }

          }
          //Transmitir (0, frequency);

      }
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

// ADC set-up function
void adc_Setup()
{
    ADC10CTL1 = ADC10DIV_7 + CONSEQ_2 + INCH_0 + ADC10SSEL_1;  // 4,096K/8 = 510Hz + ADC10SSEL_3;                      // Repeat single channel, A0
    ADC10CTL0 = ADC10SHT_1 + MSC + ADC10ON + ADC10IE;   // 510 /8 = 64Hz Sample & Hold Time + ADC10 ON + Interrupt Enable
    ADC10DTC1 = 0x40;                                   // 64 conversions
    ADC10AE0 |= 0x01;                                   // P1.0 ADC option select
}

// ADC sample conversion function
void adc_Sam10()
{
    ADC10CTL0 &= ~ENC;              // Disable Conversion
    while (ADC10CTL1 & BUSY);       // Wait if ADC10 busy
    ADC10SA = (int)adc;             // Transfers data to next array (DTC auto increments address)
    ADC10CTL0 |= ENC + ADC10SC;     // Enable Conversion and conversion start
    __bis_SR_register(CPUOFF + GIE);// Low Power Mode 0, ADC10_ISR
}
void TimerA_configure (void)
{
    TA0CCTL0 = CM_3 + CAP + SCS + CCIS1 + CCIE; // Capture on rising edge + both edges capture + sync with the clock + interrupt flag.
    __bis_SR_register(GIE); // set the global interruptions
    TA0CTL = TASSEL_2 + MC_2; // timer�s clock is system master clock, whithout division.
}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void){
    TA0CTL &= ~MC_3;    // Stop the timer (holds its value until the start)
    if(enable == 1){
        T0 = TA0CCR0; //reason: calling an interrupt and "stop timer command" demand  clock cycles.
    }else if (enable == 2){
       T1 = TA0CCR0;
       Temp_exec1 = T1 - T0;
       TA0R = 0;
   // }else if (enable == 3){
   ///     T2 = TA0CCR0;
   //     Temp_exec2 = T2 - T1;
   // } else{
   //     enable = 0;
   //     Temp_exec3 = TA0CCR0 - T0; //  clocks demanded of the interrupt function.
    }
    TA0CTL = TASSEL_2 + MC_2;   // Reconfigure timer}
    TA0CCTL0 &= ~CCIFG;
}
void setUART(void){

   P1SEL  |=  BIT1 + BIT2;  // P1.1 UCA0RXD input
   P1SEL2 |=  BIT1 + BIT2;  // P1.2 UCA0TXD output
   UCA0CTL1 |=  UCSSEL_2 + UCSWRST;  // USCI Clock = SMCLK,USCI_A0 disabled
   UCA0BR0   =  104;                 // 104 From datasheet table-
   UCA0BR1   =  0;                   // -selects baudrate =9600,clk = SMCLK
   UCA0MCTL  =  UCBRS_1;             // Modulation value = 1 from datasheet
   //UCA0STAT |=  UCLISTEN;            // loop back mode enabled
   UCA0CTL1 &= ~UCSWRST;             // Clear UCSWRST to enable USCI_A0
   IE2 |= UCA0TXIE;                  // Enable the Transmit interrupt
   IE2 |= UCA0RXIE;                  // Enable the Receive  interrupt
   _BIS_SR(GIE);                     // Enable the global interrupt

}
void Transmitir(int tempo, char sinal){

   UCA0TXBUF = sinal;                  // Transmit a byte
   while(tempo < 15000){
      tempo++;
   }

}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void TransmitInterrupt(void)
{
  //P1OUT  ^= BIT0;//light up P1.0 Led on Tx
  IFG2 &= ~UCA0TXIFG;
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void ReceiveInterrupt(void)
{
  //P1OUT  ^= BIT6;     // light up P1.6 LED on RX
  IFG2 &= ~UCA0RXIFG; // Clear RX flag
}
