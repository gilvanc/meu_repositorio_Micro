#include <msp430g2553.h>

void TimerA_configure (void);

void main()
{
      WDTCTL = WDTPW + WDTHOLD;         // Stop WDT
      BCSCTL1 |= XTS + DIVA_3; //32768
      BCSCTL2 |= DIVA_3; //32768/8 =  4,096kHz
      BCSCTL3 &= ~(LFXT1S0 + LFXT1S1);
      DCOCTL = CALDCO_1MHZ;

      while(1)
      {
          TimerA_configure();

          // Add all the sampled data and divide by 10 to find average
          //avg_adc = ((adc[0]+adc[1]+adc[2]+adc[3]+adc[4]+adc[5]+adc[6]+adc[7]+adc[8]+adc[9]) / 10);

          //for (i=0 : )if risingedge (adc)

      }
}

void TimerA_configure (void)
{
    TA0CCTL0 = CM_1 + CAP + SCS +CCIS0 +OUTMOD1; // Capture on rising edge + both edges capture + sync with the clock + interrupt flag.
   // __bis_SR_register(GIE); // set the global interruptions
   // TA0CTL = TASSEL_2 + MC_2; // timer�s clock is system master clock, whithout division.
    //TA0CCTL0 ^= CCIS0;  // Interrupt request (capture condition)
   /// TA0CTL = TASSEL_2 + MC_2;   // Reconfigure timer}
}
