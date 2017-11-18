#include <msp430g2553.h>

// Global variables
int adc[64] = {0}; //Sets up an array of 10 integers and zero's the values
//int avg_adc = 0;
volatile unsigned int enable=0, T0=0, T1=0, T2=0;
volatile unsigned int Temp_exec1 = 0,Temp_exec2 = 0, Temp_exec3 = 0;
//unsigned int frequency_avg =0;
//unsigned int wave_1=0,wave_1_n=0;
float frequency = 0;
//unsigned int wave_2=0, wave_2_n=0;
//unsigned int frequency_2=0;
//unsigned int wave_3=0, wave_3_n = 0;
//unsigned int frequency_3=0;

// Function prototypes

void adc_Setup();
void adc_Sam10();
void TimerA_configure (void);

void main()
{
      unsigned int i=0;
      WDTCTL = WDTPW + WDTHOLD;         // Stop WDT
      BCSCTL1 |= XTS + DIVA_3; //32768
      BCSCTL2 |= DIVA_3; //32768/8 =  4,096kHz
      BCSCTL3 &= ~(LFXT1S0 + LFXT1S1);
      DCOCTL = CALDCO_1MHZ;
      adc_Setup();// Fucntion call for adc_setup
      while(1)
      {
          adc_Sam10();      // Function call for adc_samp
           //Add all the sampled data and divide by 10 to find average
           //avg_adc = ((adc[0]+adc[1]+adc[2]+adc[3]+adc[4]+adc[5]+adc[6]+adc[7]+adc[8]+adc[9]) / 10);
          //frequency_avg =0;
          //wave_1=0;wave_1_n=0;
          //frequency_1=0;
          //wave_2=0; wave_2_n=0;
          //frequency_2=0;
          //wave_3=0; wave_3_n = 0;
          //frequency_3=0;
          TimerA_configure();
          for (i=1; i<65 ; i++){
              if(adc[i-1]<30 & adc[i]<800){
                  enable++;
                  TA0CCTL0 ^= CCIS0;  // Interrupt request (capture condition)
              }
              //frequency = (1/Temp_exec1)*1000;


          }
          frequency = (1000/(float)Temp_exec1);

          //for (i=0 ; i<=64 ; i++ ){
          //    if (adc[i]>800)
          //    wave_1++;
          //    else if (adc[i]==0)
          //    wave_1_n++;
          //    else if (adc[i]>800)
          //            break;
          //}
          //for (j=(i-1) ; j<=64 ; j++ ){
          //    if (adc[j]>800)
          //    wave_2++;
          //    else if (adc[j]==0)
          //    wave_2_n++;
         //     else if (adc[j]>800)
         //             break;
         // }
         // k=j;
         // for (k=(j-1) ; k<=64 ; k++ ){
         //     if (adc[k]>800)
          //    wave_3++;
          //    else if (adc[k]==0)
          //    wave_3_n++;
          //    else if (adc[k]>800)
         //             break;
         // }
         // wave_1 += wave_1_n;
         // frequency_1 = 1/wave_1;
         // wave_2 += wave_2_n;
         // frequency_2 = 1/wave_2;
         // wave_3 += wave_3_n;
          //frequency_3 = 1/wave_3;
          //frequency_avg = (frequency_1+frequency_2+frequency_3)/3;

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
        enable = 0;
   //     Temp_exec3 = TA0CCR0 - T0; //  clocks demanded of the interrupt function.
    }
    TA0CTL = TASSEL_2 + MC_2;   // Reconfigure timer}
    TA0CCTL0 &= ~CCIFG;
}