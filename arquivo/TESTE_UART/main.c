#include "msp430g2553.h"
int tempo = 0;
int j = 0;
char word[10] = {'C','O','M','P','L','E','T','E','D',' '};

void delay(void);

void Transmitir(int tempo, char sinal){

   UCA0TXBUF = sinal;                  // Transmit a byte
   delay();
   //while(tempo < 30000){
   //   tempo++;
   //}

}

void delay(void)
{
    _delay_cycles (30000);
}

void setClock(void){

  if (CALBC1_1MHZ==0xFF)   // If calibration constant erased
       {
          while(1);          // do not load, trap CPU!!
       }

     DCOCTL  = 0;             // Select lowest DCOx and MODx settings
     BCSCTL1 = CALBC1_1MHZ;   // Set range
     DCOCTL  = CALDCO_1MHZ;   // Set DCO step + modulation

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

}

void setInterrupt(void){

   IE2 |= UCA0TXIE;                  // Enable the Transmit interrupt
   IE2 |= UCA0RXIE;                  // Enable the Receive  interrupt
   _BIS_SR(GIE);                     // Enable the global interrupt

}

void main(void)
{

  WDTCTL = WDTPW + WDTHOLD; // Stop the Watch dog

  setClock();
  setUART();
  setInterrupt();

  //---------------- Configuring the LED's ----------------------//

   P1DIR  |=  BIT0 + BIT6;  // P1.0 and P1.6 output
   P1OUT  &= ~BIT0 + BIT6;  // P1.0 and P1.6 = 0

   //UCA0TXBUF = 'X';                  // Transmit a byte
   //dtostrf(test_float,7, 3, word);
   //sprintf(word, "%f", test_float);
   int tens = (97 / 10) + 48;
   int ones = (97 % 10) + 48;
   word[0] = tens;
   word[1] = ones;

   while(1){


      Transmitir(0, word[j]); //RECEPTOR PUTTY COM4
      j++;

      if(j == 11){
        break;
     }

   }

   _BIS_SR(LPM0_bits + GIE);         // Going to LPM0
}

  #pragma vector = USCIAB0TX_VECTOR
  __interrupt void TransmitInterrupt(void)
  {
    P1OUT  ^= BIT0;//light up P1.0 Led on Tx
    IFG2 &= ~UCA0TXIFG;
  }

  #pragma vector = USCIAB0RX_VECTOR
  __interrupt void ReceiveInterrupt(void)
  {
    P1OUT  ^= BIT6;     // light up P1.6 LED on RX
    IFG2 &= ~UCA0RXIFG; // Clear RX flag
  }
