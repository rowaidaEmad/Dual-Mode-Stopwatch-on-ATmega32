#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define yes 1
#define no 0
#define true 1
#define false 0
#define up 1
#define down 0

void External_interrupts_GICR(){
	GICR |= (1<<INT1) | (1<<INT0) | (1<<INT2);
}
void External_interrupts_MCUCR(){
	MCUCR &= ~(1<<ISC00);
	MCUCR|=(1<<ISC01)|(1<<ISC11)|(1<<ISC10);
}
void External_interrupts_MCUCSR(){
	MCUCSR &= ~(1<<ISC2);

}
void Internal_Timer1_init(){
	//Timer/Counter1, Output Compare A Match Interrupt Enable
	TIMSK |= (1<<OCIE1A);
	// normal mode
	TCCR1A &= ~(1<<COM1A1) &~(1<<COM1A0);
	//TCCR1A =(1<<FOC1A) ; // set at non-pwm
	//Waveform Generation Mode Bit
	TCCR1A &= ~(1<<WGM11) &~(1<<WGM10);
	TCCR1B |= (1<<WGM12);
	TCCR1B &=~(1<<WGM13);
	//max count to get interrupt each sec
	OCR1A = 15624;
	//start from
	TCNT1=0;
	//Clock Select BIT prescale 1024
	TCCR1B |=(1<<CS12)|(1<<CS10);
	TCCR1B &= ~(1<<CS11);


}
void REGs_init(){
	External_interrupts_MCUCR();
	External_interrupts_MCUCSR();
	External_interrupts_GICR();

	DDRC |= 0X0F; // decoder connected to first 4 pins(outputs) in PORTC
	PORTC = PORTC & 0XF0; // initialization with zero at Start

	DDRB = 0x00; //all push buttons connected to PORTB->inputs (resume ,mode and hours/min/sec 6 push buttons)

	DDRD &= ~(1<<PD2) & ~(1<<PD3);//inputs (reset,pause push-buttons)
	DDRD |= (1<<PD0)|(1<<PD4)|(1<<PD5); //outputs(alarm push-button,red-led,yellow-led )

	DDRA |= 0x3F;//first 6-pins in PORTA as the enable/disable pins (outputs)for the six 7-seg
	//internal pull up resistors activation for Reset(PD2) and Resume,mode and hours/min/sec 6 push buttons
	PORTD |= (1<<PD2);
	PORTB = 0xFF; //

	Internal_Timer1_init();
	//GlOBAL interrupt activation
	SREG |= (1<<7); // i-bit


}
//-----------------------------------------------------------------------------
unsigned int hr = 0 ,min = 0 ,sec = 0 , is_paused = no ,mode = up,alarm_flag=0;
//---------------------------------ISRs-----------------------------------------
ISR(INT0_vect){//reset
	hr = 0 ,min = 0 ,sec = 0 ; //reset values on screen
	alarm_flag=0;
	TCNT1=0; //resent timer1 register
	GIFR |= (1<<INTF0);
}
ISR(INT1_vect){//pause
	is_paused = yes;
	alarm_flag=0;
	//disable timer clk
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10)); // stop timer
	GIFR |= (1<<INTF1);
}
ISR(INT2_vect){//resume
	is_paused = no;
	//enable timer clk
	TCCR1B |=(1<<CS12)|(1<<CS10);
	TCCR1B &= ~(1<<CS11);
	GIFR |= (1<<INTF2);
}
ISR(TIMER1_COMPA_vect){
	if(mode == up){
		sec++;
		if(sec > 59){
			sec=0;
			min++;
		}
		if(min>59){
			min=0;
			hr++;
		}
		if(hr>99){
			hr=99;min=59;sec=59;
		}
	}
	else if (mode!=up){
		if(sec > 0){
			sec--;
		}
		else if(sec ==0 && min!=0){
			min--;
			sec =59;
		}
		else if (sec == 0 && min ==0 && hr!=0){
			hr--;
			min=59;
			sec=59;
		}
		else if(sec==0 && min ==0 && hr==0){
			alarm_flag=1; //alarm on
		}
	}
}

void Display_digits(void){
	//PORTA->Enable , PORTC->decoder
    PORTA &= ~0x3F;   // all off
    ///SEGMENT of hour tens
    PORTC = (PORTC & 0xF0) | (hr/10);
    PORTA = (1<<0);
    _delay_ms(1);

    // /SEGMENT of hour units
    PORTA &= ~0x3F;
    PORTC = (PORTC & 0xF0) | (hr%10);
    PORTA = (1<<1);
    _delay_ms(1);

    //SEGMENT of minute tens
    PORTA &= ~0x3F;
    PORTC = (PORTC & 0xF0) | (min/10);
    PORTA = (1<<2);
    _delay_ms(1);

    ///SEGMENT of minute units
    PORTA &= ~0x3F;
    PORTC = (PORTC & 0xF0) | (min%10);
    PORTA = (1<<3);
    _delay_ms(1);

    ///SEGMENT of second tens
    PORTA &= ~0x3F;
    PORTC = (PORTC & 0xF0) | (sec/10);
    PORTA = (1<<4);
    _delay_ms(1);

    ///SEGMENT of second units
    PORTA &= ~0x3F;
    PORTC = (PORTC & 0xF0) | (sec%10);
    PORTA = (1<<5);
    _delay_ms(1);


}
//flags to handle buttons push/release
unsigned int hr_inc_flag=0,hr_dec_flag=0,min_inc_flag=0,min_dec_flag=0,sec_inc_flag=0,sec_dec_flag=0;
int main(void){
	REGs_init();
 while(1){
	 Display_digits();
	 if(is_paused){
		 //mode toggle handling
		 if(!(PINB & (1<<PB7))){
			 if(mode != down){
				 mode = down;
			 }
		 }
		 else{
			 mode = up;
		 }
		 //inc-dec handling
		 if (!(PINB & (1<<PB1))){ //inc hr
			 if(hr_inc_flag==0){
				 if(hr < 99) hr++;
				 hr_inc_flag=1;
			 }

		 }
		 else {
			 hr_inc_flag=0;//reset flag
		 }

		 if (!(PINB & (1<<PB0))){ //dec hr
			 if(hr_dec_flag==0){
				 if(hr>0) hr--;
				 hr_dec_flag=1;
			 }
		 }
		 else{
			 hr_dec_flag=0;
		 }

		 if (!(PINB & (1<<PB4))){//inc min
			 if(min_inc_flag==0){
				 if(min < 59) min++;
				 min_inc_flag=1;
			 }
		 }
		 else{
			 min_inc_flag=0;
		 }

		 if (!(PINB & (1<<PB3))){ //dec min
			 if(min_dec_flag==0){
				 if(min>0) min--;
				 min_dec_flag=1;
			 }
		 }
		 else{
			 min_dec_flag=0;
		 }

		 if (!(PINB & (1<<PB6))){//inc sec
			 if(sec_inc_flag==0){
				 if(sec < 59) sec++;
				 sec_inc_flag=1;
			 }
		 }
		 else{
			 sec_inc_flag=0;
		 }

		 if (!(PINB & (1<<PB5))){ //dec sec
			 if(sec_dec_flag==0){
				 if(sec>0) sec--;
				 sec_dec_flag=1;
			 }
		 }
		 else{
			 sec_dec_flag=0;
		 }

	}
	if(alarm_flag){
		 PORTD |=(1<<PD0);
	 }
	else if(!alarm_flag){
		PORTD &= ~(1<<PD0);
	}//LEDs handling for count down and count up
	if(mode == down){
		PORTD|=(1<<PD5);
		PORTD &=~(1<<PD4);
	}
	else{
		PORTD|=(1<<PD4);
		PORTD &=~(1<<PD5);
	}

}
}



