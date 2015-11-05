#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include "HD44780.h"
#include "i2chw/i2cmaster.h"
#include "ds1307.h"
#include <avr/interrupt.h>

#define PREV_BUTTON_PORT 	PORTD
#define PREV_BUTTON_PIN 	PIND
#define PREV_BUTTON_BIT 	PD4

#define NEXT_BUTTON_PORT 	PORTD
#define NEXT_BUTTON_PIN		PIND
#define NEXT_BUTTON_BIT		PD5

#define BACK_BUTTON_PORT 	PORT
#define BACK_BUTTON_PIN		PIND
#define BACK_BUTTON_BIT		PD6

#define SEL_BUTTON_PORT 	PORTD
#define SEL_BUTTON_PIN		PIND
#define SEL_BUTTON_BIT		PD7

#define MAX_OPTIONS			5
#define BTN_BOUNCE 			20

static void display_opt(unsigned char);
static void check_buttons(void);

unsigned char item = 0, item_old = 0;

//menu options
char menu_opt[MAX_OPTIONS][15] = { "Pokaz czas",
								   "Ustaw czas",
								   "Ustaw date",
								   "Ustaw alarm",
								   "Wl/Wyl alarm"
};

int main(void) {
	PORTD = 0xF0;
	sei();
	uint8_t year = 0;
	uint8_t month = 0;
	uint8_t day = 0;
	uint8_t hour = 0;
	uint8_t minute = 0;
	uint8_t second = 0;
	ds1307_setdate(15, 11, 06, 18, 21, 10);

	LCD_Initalize();
	LCD_Clear();
	LCD_Home();
	LCD_WriteText("> ");
	LCD_WriteText(&menu_opt[item][0]);
	LCD_GoTo(2,1);
	LCD_WriteText(&menu_opt[item+1][0]);
	while (1){
		ds1307_getdate(&year, &month, &day, &hour, &minute, &second);
		char buf[50];
		sprintf(buf, "%d/%d/%d %d:%d:%d", year, month, day, hour, minute, second);
		LCD_WriteText(&buf);
		_delay_ms(1000);
		LCD_Clear();
		LCD_GoTo(0,0);
	}
}

//Display options
void display_opt(unsigned char item) {
	if(item > item_old) {
		LCD_Clear();
		LCD_GoTo(2,0);
		LCD_WriteText(&menu_opt[item-1][0]);
		LCD_GoTo(0,1);
		LCD_WriteText("> ");
		LCD_WriteText(&menu_opt[item][0]);
	}
	else if(item < item_old) {
		LCD_Clear();
		LCD_GoTo(0,0);
		LCD_WriteText("> ");
		LCD_WriteText(&menu_opt[item][0]);
		LCD_GoTo(2,1);
		LCD_WriteText(&menu_opt[item+1][0]);
	}
}

void check_buttons(void) {
	//previous button pressed
	if(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){
		_delay_ms(BTN_BOUNCE);
		while(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){};
		item_old = item;
		item -= item<1 ? 0 : 1;
		display_opt(item);
	}

	//next button pressed
	if(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {
		_delay_ms(BTN_BOUNCE);
		while(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {};
		item_old = item;
		item += item>(MAX_OPTIONS-2) ? 0 : 1;
		display_opt(item);
		_delay_ms(BTN_BOUNCE);
	}

	//select button pressed
	if(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){
		_delay_ms(BTN_BOUNCE);
		while(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){};
		LCD_Clear();
		LCD_WriteText("Selected opt: ");
		LCD_GoTo(0,1);
		LCD_WriteText(&menu_opt[item][0]);
		_delay_ms(BTN_BOUNCE);
	}

	//back button pressed
	if(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)){
		_delay_ms(BTN_BOUNCE);
		while(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)) {};
		if(item < (MAX_OPTIONS-1)){
			LCD_Clear();
			LCD_GoTo(0,0);
			LCD_WriteText("> ");
			LCD_WriteText(&menu_opt[item][0]);
			LCD_GoTo(2,1);
			LCD_WriteText(&menu_opt[item+1][0]);
		}else {
			LCD_Clear();
			LCD_GoTo(2,0);
			LCD_WriteText(&menu_opt[item-1][0]);
			LCD_GoTo(0,1);
			LCD_WriteText("> ");
			LCD_WriteText(&menu_opt[item][0]);
		}
		_delay_ms(BTN_BOUNCE);
	}
}
