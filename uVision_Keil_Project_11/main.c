#include "TM4C123.h"
#include <stdint.h>
#include <stdio.h>

// LCD Macros and Global Variables
#define LCD GPIOB      // LCD port with Tiva C (PB4-PB7)
#define RS 0x10        // RS -> PB4
#define RW 0x20        // RW -> PB5
#define EN 0x40        // EN -> PB6

unsigned volatile long j;
char buffer[60];
volatile int rpm = 0;               // Global RPM counter
volatile int interruptOccurred = 0; // Interrupt flag
volatile int updateFlag = 0;        // Update flag for periodic display
volatile float current_value = 0.0;
// Function Declarations
void UART0_SendString(char *str);
void PLL_Init(void);
void PF2_as_M1PWM6_Init(void);
void PWM_Module1_Channel6_Init(void);

void UART0_Tx_RX_Init(void);
void PA0_1_as_UART_Tx_Rx_Init(void);
void delay(long d);
void delayMs(int);
void delayUs(int);
void floatToString(float num) ;
void LCD4bits_Init(void);
void LCD_Write4bits(unsigned char, unsigned char);
void LCD_WriteString(char*);
void LCD4bits_Cmd(unsigned char);
void LCD4bits_Data(unsigned char);
void intToString(int num);
void PF2_Digital_Output_Init(void);
void PE0_External_Interrupt_Input_Init(void);
void GPIOE_Handler(void);
void Display_RPM_Current(int rpm, int current);
void ADC_Enable(void);

// Main Function
int main(void)
{
    PLL_Init();
	    PA0_1_as_UART_Tx_Rx_Init();
PF2_as_M1PWM6_Init();
	PWM_Module1_Channel6_Init();
    UART0_Tx_RX_Init();
    LCD4bits_Init();                // Initialize LCD
   // PF2_Digital_Output_Init();      // Initialize PF2 as digital output
    PE0_External_Interrupt_Input_Init(); // Initialize PE0 for external interrupt
    ADC_Enable();
    while (1)
    {
				
			
    		// Start ADC conversion for PE3 (Potentiometer)
		ADC0->PSSI |= 0x08;
		while ((ADC0->RIS & 8) == 0);
		 int adc_value_pot= ADC0->SSFIFO3;
         ADC0->ISC = 8;    
         		// Calculate duty cycle based on potentiometer input (PE3)
		float voltage_pot = (float)adc_value_pot / 4095.0;
		int duty = (voltage_pot * 250) + 250; 
PWM1->_3_CMPA = duty;
		// Start ADC conversion for PE1 (Current Sensor)
		ADC1->PSSI |= (1 << 3);
		while ((ADC1->RIS & (1 << 3)) == 0);
		int adc_value_current = ADC1->SSFIFO3;
		ADC1->ISC = (1 << 3);

		// Calculate current from ADC value (PE1)
		// AcsValueF = (2.56 - (AvgAcs * (5.12 / 1024.0)) )/0.066;
		float voltage_current = (float)adc_value_current * (4.7/ 4095.0);
		 current_value = (voltage_current-3.65 ) / 0.1;
			if (current_value < 0){
			current_value = -current_value;}

            // Simulate Current (for demonstration)
            int current = rpm * 30; // Example calculation for current
                LCD4bits_Cmd(0x01); // Clear display screen
    LCD4bits_Cmd(0x80); // Move cursor to the beginning of 1st line

    LCD_WriteString("RPM: ");
    intToString(current);
    LCD_WriteString(buffer);

    LCD4bits_Cmd(0xC0); // Move cursor to the beginning of 2nd line
    LCD_WriteString("CURRENT: ");
 floatToString(current_value);
      LCD_WriteString(buffer);
            updateFlag = 0;  // Reset flag
			 
             // Convert RPM to a string and send over UART
    char rpm_string[20];
			sprintf(rpm_string, "RPM: %d, DUTY : %d , Current :%.2f\r\n", current,duty,current_value);
    UART0_SendString(rpm_string);// Send RPM string over UART
        // Check if the interrupt occurred in the last second
        if (!interruptOccurred)
        {
            rpm = 0; // Reset RPM if no new pulse detected
        }

        interruptOccurred = 0; // Reset the flag for the next cycle
        delay(1000000);       // 1s delay
				
    }
}

// Function Definitions

void delay(long d)
{
    for (long i = 0; i < d; i++);
}



void PE0_External_Interrupt_Input_Init()
{
    SYSCTL->RCGCGPIO |= 0x10;  // Enable clock for Port E
    for (j = 0; j < 3; j++);   // Delay for clock to stabilize

    GPIOE->AFSEL &= ~0x01;     // Disable alternate functions on PE0
    GPIOE->DEN |= 0x01;        // Enable digital functionality on PE0
    GPIOE->DIR &= ~0x01;       // Set PE0 as an input pin

    GPIOE->IS &= ~0x01;        // Edge-triggered interrupt
    GPIOE->IEV &= ~0x01;       // Falling edge trigger
    GPIOE->IBE &= ~0x01;       // Disable both edge trigger
    GPIOE->IM |= 0x01;         // Enable interrupt for PE0
    GPIOE->ICR |= 0x01;        // Clear interrupt flags

    NVIC->ISER[0] |= (1 << 4); // Enable IRQ for Port E (Interrupt number 4)
    NVIC->IP[1] = 0x00200000;  // Set priority to 1
}

void GPIOE_Handler(void)
{
    if ((GPIOE->MIS & 0x01) == 0x01) // Check if interrupt occurred on PE0
    {
        rpm++;                        // Increment RPM
        if (rpm > 9999) rpm = 0;      // Wrap RPM to avoid overflow
        interruptOccurred = 1;        // Set interrupt flag
        for (j = 0; j < 1000; j++);   // Delay to debounce
        GPIOE->ICR |= 0x01;           // Clear interrupt flag
    }
}



// Other helper functions (LCD functions, intToString, etc.) remain the same.
// LCD Initialization function
void LCD4bits_Init(void)
{
    SYSCTL->RCGCGPIO |= 0x02;    // Enable clock for PORTB
    delayMs(10);                 // Delay 10 ms for enable the clock of PORTB
    LCD->DIR = 0xFF;             // Let PORTB as output pins
    LCD->DEN = 0xFF;             // Enable PORTB digital IO pins
    LCD4bits_Cmd(0x28);          // 2 lines and 5x7 character (4-bit data, D4 to D7)
    LCD4bits_Cmd(0x06);          // Automatic Increment cursor (shift cursor to right)
    LCD4bits_Cmd(0x01);          // Clear display screen
    LCD4bits_Cmd(0x0C);          // Display on, cursor blinking
}

void LCD_Write4bits(unsigned char data, unsigned char control)
{
    data = (data >> 4) & 0x0F;           // Extract upper nibble
    control &= 0xF0;                    // Clear lower nibble for control
    LCD->DATA = (data | control);       // Combine data and control bits
    LCD->DATA |= EN;                    // Pulse EN
    delayUs(0);                         // Short delay for EN pulse
    LCD->DATA &= ~EN;                   // Clear EN pulse
    delayUs(40);                        // Small delay for stabilization
}


// Function to write a string to LCD
void LCD_WriteString(char * str)
{  
    volatile int i = 0;          // Volatile is important 
    while(*(str+i) != '\0')       // Until the end of the string
    {
        LCD4bits_Data(*(str+i));    // Write each character of the string
        i++;                        // Increment for next character
    }
}

// Function to send command to LCD
void LCD4bits_Cmd(unsigned char command)
{
    LCD_Write4bits(command & 0xF0 , 0);    // Upper nibble first
    LCD_Write4bits(command << 4 , 0);       // Then lower nibble
    
    if(command < 4)
        delayMs(2);       // Commands 1 and 2 need up to 1.64ms
    else
        delayUs(40);      // All others 40 us
}

// Function to send data to LCD
void LCD4bits_Data(unsigned char data)
{
    LCD_Write4bits(data & 0xF0 , RS);   // Upper nibble first
    LCD_Write4bits(data << 4 , RS);     // Then lower nibble
    delayUs(40);                        // Delay for LCD (MCU is faster than LCD)
}

// Delay functions
void delayMs(int n)
{  
    volatile int i,j;             // Volatile is important for variables incremented in code
    for(i=0;i<n;i++)
        for(j=0;j<3180;j++)         // Delay for 1 ms
        {}
}

void delayUs(int n)             
{
    volatile int i,j;             // Volatile is important for variables incremented in code
    for(i=0;i<n;i++)
        for(j=0;j<3;j++)            // Delay for 1 microsecond
        {}
}
void intToString(int num) {
    int index = 0;

    // Handle 0 case separately
    if (num == 0) {
        buffer[index++] = '0';
    }

    // Extract digits in reverse order
    while (num > 0) {
        buffer[index++] = '0' + (num % 10);
        num /= 10;
    }

    buffer[index] = '\0';

    // Reverse the buffer to get the correct order
    for (int i = 0; i < index / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[index - i - 1];
        buffer[index - i - 1] = temp;
    }
}
void ADC_Enable(void)
{
    // Enable Clock for GPIOE and ADC0/ADC1
    SYSCTL->RCGCGPIO |= (1 << 4);  // Enable clock for Port E
    SYSCTL->RCGCADC |= 0x03;       // Enable ADC0 and ADC1

    // Configure PE3 (Potentiometer - AIN0)
    GPIOE->AFSEL |= (1 << 3);      // Enable alternate function on PE3
    GPIOE->DEN &= ~(1 << 3);       // Disable digital function on PE3
    GPIOE->AMSEL |= (1 << 3);      // Enable analog mode on PE3

    // Configure PE2 (Current Sensor - AIN1)
    GPIOE->AFSEL |= (1 << 2);      // Enable alternate function on PE2
    GPIOE->DEN &= ~(1 << 2);       // Disable digital function on PE2
    GPIOE->AMSEL |= (1 << 2);      // Enable analog mode on PE2

    // Configure ADC0 for PE3 (Potentiometer - AIN0)
    ADC0->ACTSS &= ~(1 << 3);      // Disable sample sequencer 3 during configuration
    ADC0->EMUX &= ~0xF000;         // Software trigger conversion
    ADC0->SSMUX3 = 0;              // AIN0 (PE3)
    ADC0->SSCTL3 |= 0x06;          // Enable interrupt and end-of-sequence
    ADC0->ACTSS |= (1 << 3);       // Enable sample sequencer 3

    // Configure ADC1 for PE2 (Current Sensor - AIN1)
    ADC1->ACTSS &= ~(1 << 3);      // Disable sample sequencer 3 during configuration
    ADC1->EMUX &= ~0xF000;         // Software trigger conversion
    ADC1->SSMUX3 = 1;              // AIN1 (PE2)
    ADC1->SSCTL3 |= 0x06;          // Enable interrupt and end-of-sequence
    ADC1->ACTSS |= (1 << 3);       // Enable sample sequencer 3
}
void floatToString(float num) {
    int integer_part = (int)num;  // Get the integer part
    int decimal_part = (int)((num - integer_part) * 100);  // Get the decimal part (up to 2 decimal places)

    int index = 0;

    // Handle the integer part first (same as your intToString logic)
    if (integer_part == 0) {
        buffer[index++] = '0';
    } else {
        int temp = integer_part;
        // Extract digits in reverse order
        while (temp > 0) {
            buffer[index++] = '0' + (temp % 10);
            temp /= 10;
        }

        // Reverse the buffer to get the correct order
        for (int i = 0; i < index / 2; i++) {
            char temp_char = buffer[i];
            buffer[i] = buffer[index - i - 1];
            buffer[index - i - 1] = temp_char;
        }
    }

    // Add the decimal point if there's a decimal part
    buffer[index++] = '.';

    // Handle the decimal part (up to 2 decimal places)
    if (decimal_part < 0) decimal_part = -decimal_part;  // Ensure the decimal part is positive
    buffer[index++] = '0' + (decimal_part / 10);  // Get the first decimal digit
    buffer[index++] = '0' + (decimal_part % 10);  // Get the second decimal digit

    buffer[index] = '\0';  // Null-terminate the string
}

void UART0_SendString(char *str)
{
	while (*str)
	{
		while ((UART0->FR & 0x20) != 0)
			; // Wait until the transmit FIFO is not full
		UART0->DR = *str;
		str++;
	}
}
// Configure the system clock to 16 MHz using PLL-------------------------------
void PLL_Init(void)
{

	// 0) Use RCC2
	SYSCTL->RCC2 |= 0x80000000; // USERCC2

	// 1) bypass PLL while initializing
	SYSCTL->RCC2 |= 0x00000800; // BYPASS2, PLL bypass

	// 2) select the crystal value and oscillator source
SYSCTL->RCC = (SYSCTL->RCC & ~0x000007C0) + 0x00000540;
// clear XTAL field, bits 10-6  // 10101, configure for 16 MHz crystal
	SYSCTL->RCC2 &= ~0x00000070;							// configure for main oscillator source

	// 3) activate PLL by clearing PWRDN
	SYSCTL->RCC2 &= ~0x00002000;

	// 4) set the desired system divider
	SYSCTL->RCC2 |= 0x40000000;								  // use 400 MHz PLL
	SYSCTL->RCC2 = (SYSCTL->RCC2 & ~0x1FC00000) + (24 << 22); // configure for 16 MHz clock / / clear system clock divider +

	// 5) wait for the PLL to lock by polling PLLLRIS
	while ((SYSCTL->RIS & 0x00000040) == 0)
	{
	} // wait for PLLRIS bit

	// 6) enable use of PLL by clearing BYPASS
	SYSCTL->RCC2 &= ~0x00000800;
}

void PF2_as_M1PWM6_Init(void){
	// Step 1: Clock enable on PortF
	SYSCTL->RCGCGPIO |= 0x20;		// 0b 0010 0000 
	for (j =0; j < 20 ; j++)			// at least 3 clock cyles
	
	// Step 2: APB is selected for PortF by selecting
	// 0x40025000 as Base Address in DATA section
	
	// Step 3: Enable alternate functionality on PortF
	GPIOF->AFSEL |= 1<<2;				// 0b 0000 0100
	
	// Step 4: Enable digital pin functionaliy on PortF pin 2
	GPIOF->DEN |= 1<<2; // Digital enable for PF2
	
	// Step 5: Set PortF pin 2 as an output pin
	GPIOF->DIR |= 1<<2; // PF2 as Output pin
	
	// Step 6: Configure PortF pin 2 as M1PWM6 pin (Table 10-2 of Datasheet, page # 651)
	GPIOF->PCTL &= 0xFFFFF0FF;		// clear the bit fields
	GPIOF->PCTL |= 0x00000500;
}

void PWM_Module1_Channel6_Init(void){
	
	// Step 1: Clock Gating Control of PWM Module 1
	SYSCTL->RCGCPWM |= (1<<1);		// b 0000 0010
	for (j =0; j < 20 ; j++)			// at least 3 clock cyles
	SYSCTL->RCC	|= ((1<<20));		// disable clock signal divisor for PWM
	
	// Step 2: For PWM Channel configurations
	// we need check which PWM block our PWM Pin blongs to. For our case PF2
	// is M1PWM6 which is a part of PWM block 3 
	// Read any register description for identifying the block, e.g., CTL
	
	// Step 3: Disable Generator 3 before performing configurations
	// Step 4: Select Counter 3 Count-down mode
	PWM1->_3_CTL = 0x00;
	
	// Step 5: Set Load value for 10 kHz
	// (16 MHz / 10 kHz = 1600)
	PWM1->_3_LOAD = 5000;
	
	// Step 6: Set Compare Register Value to set 50% duty cycle
	// 50% of Load value = 1600 x 50% = 1600 x 0.5 = 800
	PWM1->_3_CMPA = 250;
	
	// Step 7: Perform Generating Control Register configuration
	// PWM signal HIGH when counter relaoded and LOW when matches CMPA Value
	PWM1->_3_GENA |= 0xC8;
	
	// Step 8: Enable generator 3 counter
	PWM1->_3_CTL |= 0x01;
	
	// Step 9: Enalbe PWM Module 1 Channel 6 Output
	PWM1->ENABLE |= 1<<6;
}

void UART0_Tx_RX_Init(void){
	// UART0 -> PA0 (Rx), PA1	(Tx) (Virtual COM Port available on TIVA)
	// If we use any other UART module then we will need to use
	// TTL converter or UART to USB converter while connecting to PC
	
	// Step 1: Enable Clock Gating Control
	SYSCTL->RCGCUART |= 0x01;			// Enable clock on UART0
	for (j =0; j < 3 ; j++);		// at least 3 clock cyles
	
	// Step 2: Ensure UART is disabled before performing configurations
	UART0->CTL = 0x00;		// Disable UART, Tx, Rx
	
	// Step 3: Set UART Baud Rate
	UART0->IBRD = 104;		// BRD = UART_Sysclk/(ClkDiv*Buad Rate)
												// BRD = 16,000,000/(16*9600) = 104.166666667
	UART0->FBRD = 11;			// FBRD = BRDF *64+0.5 = 0.166666667*64+0.5
												// FBRD = 11.1666666667
	
	// Step 4: Perform Line Configurations
	UART0->LCRH |= 0x60;	// 8-bit word length
	
	// Step 5: Select Clock Source
	UART0->CC |= 0x05; 		// Use PIOSC as a source for UART Baud clock
	
	// Step 6: Turn On UART Module
	UART0->CTL |= 0x301;	// Turn on UART EN & TxEN & RxEN bits
}

void PA0_1_as_UART_Tx_Rx_Init(void){
	// Step 1: Clock enable on PortF
	SYSCTL->RCGCGPIO |= 0x01;		// 0b 0000 0001
	for (j =0; j < 3 ; j++);			// at least 3 clock cyles
	
	// Step 2: APB is selected for PortA by selecting
	// 0x40025000 as Base Address in DATA section
	
	// Step 3: Enable alternate functionality on PortA
	GPIOA->AFSEL |= 0x03;				// 0b 0000 0011
	
	// Step 4: Enable digital pin functionaliy on PortA pin0-1 
	GPIOA->DEN |= 0x03; // Digital enable for PA0-1
	
	// Step 5: Set PortA pin 0-1 as input/output pins
	GPIOA->DIR &= ~0x01; // PA0 as Input pin (Rx)
	GPIOA->DIR |= 0x02;  // PA1 as Output pin (Tx)
	
	// Step 6: Configure PortA pin 0-1 as UART0 pin (Table 14-2 of Datasheet, page # )
	GPIOA->PCTL &= 0xFFFFFF00;		// clear the bit fields
	GPIOA->PCTL |= 0x00000011;
}