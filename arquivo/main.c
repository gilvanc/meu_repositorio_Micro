//
/// Trabalho final da disciplina Microcontroladores e Miprocessadores - FGA
/// Professor: Diogo Caetano
/// Autores: Gilvan Camargo 14/0141537
///          Maria Carolina 14/0153403
///
///            Batimento Card�aco com Ox�metro
/// Amostragem de sinal card�aco e transmiss�o de batimentos po minuto


#include "msp430g2553.h"


// Global variables
unsigned int i=0; // Contador
volatile unsigned int enable=0, T0=0, T1=0; // Verificam in�cio e final do pulso card�aco
volatile unsigned int period_in = 0, period_out = 0, bpm = 0; // Calculam per�odo do pulso
int adc[64] = {0}; //Vetor para armazenar amostras
int c = 0, d = 0, u = 0; // centenas, dezenas e unidades para armazena o "bpm"
char word[8] = "CDU BPM"; // Caracteres para BPM


// Function prototypes
void setClock(void);
void adc_Setup(void);
void adc_Sam64(void);
void setUARTInterrupt(void);
void setUART(void);
void TimerA_configure (void);
void Transmitir(volatile unsigned char sinal);
void get_bpm(void);
void write_on_word (void);



void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop the Watch dog
    setClock();
    adc_Setup();// Fucntion call for adc_setup
    setUART();
    while (1)
    {
        i=0;
        adc_Sam64();      // Function call for adc_samp
        TimerA_configure ();
        get_bpm();
        write_on_word();
        setUARTInterrupt();
        while(1)
            {
                Transmitir(word[i]); //RECEPTOR PUTTY COM5
                i++;

                if(i >= 7){
                    i=0;
                    break;
                }
            }
    }

}

void write_on_word (void)
{
    //Write Digits  on string with ANSI
    c = (bpm / 100) + 48;
    if (bpm < 100)
        d = (bpm / 10) + 48;
    else {
        d = bpm;
        while (d>100){
            d -= 100;
        }
        d = d/10 +48;
    }
    u = (bpm % 10) + 48;
    word[0] = c;
    word[1] = d;
    word[2] = u;
    word[7] = '\n';
}

void get_bpm(void)
{
    i=0;
    enable = 0;
    period_in = period_out;
    for (i=1; i<65 ; i++)
    {
        if (enable ==2)
            break;
        if(adc[i-1]<30 & adc[i]>800){
            enable++;
            TA0CCTL0 ^= CCIS0;  // Interrupt request (capture condition)
            }
    }
    i=0;
    if (period_in == period_out)
        bpm = 0;
    else
        bpm = (600000/period_out);
}

void setUARTInterrupt(void)
{
   IE2 |= UCA0TXIE;                  // Enable the Transmit interrupt
   IE2 |= UCA0RXIE;                  // Enable the Receive  interrupt
   _BIS_SR(GIE);                     // Enable the global interrupt

}

void setClock(void)
{
    DCOCTL  = 0;             // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ +XTS + DIVA_3;   // Set range
    BCSCTL2 |= DIVA_3; //32768/8 =  4,096kHz
    DCOCTL  = CALDCO_1MHZ;   // Set DCO step + modulation
}

// ADC set-up function
void adc_Setup(void)
{
    ADC10CTL1 = ADC10DIV_7 + CONSEQ_2 + INCH_0 + ADC10SSEL_1;  // 4,096K/8 = 510Hz + ADC10SSEL_3;                      // Repeat single channel, A0
    ADC10CTL0 = ADC10SHT_1 + MSC + ADC10ON + ADC10IE;   // 510 /8 = 64Hz Sample & Hold Time + ADC10 ON + Interrupt Enable
    ADC10DTC1 = 0x40;                                   // 64 conversions
    ADC10AE0 |= 0x01;                                   // P1.0 ADC option select
}

// ADC sample conversion function
void adc_Sam64(void)
{
    ADC10CTL0 &= ~ENC;              // Disable Conversion
    while (ADC10CTL1 & BUSY);       // Wait if ADC10 busy
    ADC10SA = (int)adc;             // Transfers data to next array (DTC auto increments address)
    ADC10CTL0 |= ENC + ADC10SC;     // Enable Conversion and conversion start
    __bis_SR_register(CPUOFF + GIE);// Low Power Mode 0, ADC10_ISR
}

void Transmitir(volatile unsigned char sinal)
{
    UCA0TXBUF = sinal;                  // Transmit a byte
    _delay_cycles (5000);
}

void setUART(void)
{
   P1SEL  |=  BIT1 + BIT2;  // P1.1 UCA0RXD input
   P1SEL2 |=  BIT1 + BIT2;  // P1.2 UCA0TXD output
   UCA0CTL1 |=  UCSSEL_2 + UCSWRST;  // USCI Clock = SMCLK,USCI_A0 disabled
   UCA0BR0   =  104;                 // 104 From datasheet table-
   UCA0BR1   =  0;                   // -selects baudrate =9600,clk = SMCLK
   UCA0MCTL  =  UCBRS_1;             // Modulation value = 1 from datasheet
   //UCA0STAT |=  UCLISTEN;            // loop back mode enabled
   UCA0CTL1 &= ~UCSWRST;             // Clear UCSWRST to enable USCI_A0
}

void TimerA_configure (void)
{
    TA0CCTL0 = CM_3 + CAP + SCS + CCIS1 + CCIE; // Capture on rising edge + both edges capture + sync with the clock + interrupt flag.
    __bis_SR_register(GIE); // set the global interruptions
    TA0CTL = TASSEL_2 + MC_2; // timer�s clock is system master clock, whithout division.
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

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void){
    TA0CTL &= ~MC_3;    // Stop the timer (holds its value until the start)
    if(enable == 1){
        T0 = TA0CCR0; //reason: calling an interrupt and "stop timer command" demand  clock cycles.
    }else if (enable == 2){
       T1 = TA0CCR0;
       period_out = T1 - T0;
       TA0R = 0;
    }
    TA0CTL = TASSEL_2 + MC_2;   // Reconfigure timer}
    TA0CCTL0 &= ~CCIFG;
}
