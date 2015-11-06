#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>
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
static void show_time(void);
static void set_time(void);
static void set_date(void);

unsigned char item = 0, item_old = 0;
const uint8_t ds1307_daysinmonths [] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
//menu options
char menu_opt[MAX_OPTIONS][15] = { "Pokaz czas",
								   "Ustaw czas",
								   "Ustaw date",
								   "Ustaw alarm",
								   "Wl/Wyl alarm"
};

uint8_t year = 0;
uint8_t month = 0;
uint8_t day = 0;
uint8_t hour = 0;
uint8_t minute = 0;
uint8_t second = 0;

int main(void) {
	PORTD = 0xF0;

	//ds1307_setdate(15, 11, 06, 18, 58, 00);

	LCD_Initalize();
	LCD_Clear();
	LCD_Home();
	LCD_WriteText("> ");
	LCD_WriteText(&menu_opt[item][0]);
	LCD_GoTo(2,1);
	LCD_WriteText(&menu_opt[item+1][0]);
	while (1){
		//show_time();
		check_buttons();
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
		if (item == 0) {
			show_time();
		};
		if (item == 1) {
			set_time();
		}
		if (item==2) {
			set_date();
		}
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

void show_time(void) {
	int zm = 0;
	while(zm==0) {
			ds1307_getdate(&year, &month, &day, &hour, &minute, &second);
			char buf[50];
			char buf1[50];
			LCD_Clear();
			LCD_GoTo(2,0);
			sprintf(buf, "%d/%d/%d", year, month, day);
			LCD_WriteText(&buf[0]);
			sprintf(buf1, "%d:%d:%d", hour, minute, second);
			LCD_GoTo(4,1);
			LCD_WriteText(&buf1[0]);
			_delay_ms(200);
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
					zm=1;
				}else {
					LCD_Clear();
					LCD_GoTo(2,0);
					LCD_WriteText(&menu_opt[item-1][0]);
					LCD_GoTo(0,1);
					LCD_WriteText("> ");
					LCD_WriteText(&menu_opt[item][0]);
					zm=1;
				}
				_delay_ms(BTN_BOUNCE);
			}
	}
}

void set_time(void) {
	ds1307_getdate(&year, &month, &day, &hour, &minute, &second);

	uint8_t n_hour = hour;
	uint8_t n_minute = minute;
	uint8_t n_second = second;

	char bufor[50];
	sprintf(bufor, "%d:%d:%d", n_hour, n_minute, n_second);

	LCD_Clear();
	LCD_GoTo(0,0);
	LCD_WriteText(&bufor[0]);
}

void set_date(void) {
	ds1307_getdate(&year, &month, &day, &hour, &minute, &second);

	uint8_t n_year = year;
	uint8_t n_month = month;
	uint8_t n_day = day;

	int zmi = 0;
	int wyb = 0;
	while(zmi==0) {
		char bufor[50];
		sprintf(bufor, "%d/%d/%d", n_year, n_month, n_day);
		LCD_Clear();
		LCD_GoTo(0,0);
		LCD_WriteText(&bufor[0]);
		if (wyb==0){
			LCD_GoTo(0,1);
			LCD_WriteText("Ustaw rok");}
		if (wyb==1){
			LCD_GoTo(0,1);
			LCD_WriteText("Ustaw miesiac");}
		if (wyb==2){
		LCD_GoTo(0,1);
		LCD_WriteText("Ustaw dzien");
		}

		if(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){};
			if (wyb==0) {
				n_year = n_year - 1;
				year = n_year;
			}
			if (wyb==1) {
				n_month = n_month - 1;
				month = n_month;
			}
			if (wyb==2) {
				n_day = n_day -1;
				day = n_day;
			}
			_delay_ms(BTN_BOUNCE);
		}

		if(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {};
			if (wyb==0) {
				n_year = n_year + 1;
				year = n_year;
			}
			if (wyb==1) {
				if (n_month < 12) {
					n_month = n_month + 1;
					month = n_month;
				}
				else {
					n_month = 1;
				}

			}
			if (wyb==2) {
				if (n_day < ds1307_daysinmonths[month]) {
				n_day = n_day + 1;
				day = n_day; }
				else n_day = 1;
			}
			_delay_ms(BTN_BOUNCE);
		}

		if(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){};
			year = n_year;
			_delay_ms(BTN_BOUNCE);
			wyb=wyb+1;
			if (wyb >= 3) {
				zmi=1;
			}
		}

		if(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)){
			zmi=1;
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
		_delay_ms(100);

	}

	ds1307_setdate(year, month, day, hour, minute, second);

}
