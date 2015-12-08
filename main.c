#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include "HD44780.h"
#include "i2chw/i2cmaster.h"
#include "ds1307.h"
#include <avr/interrupt.h>
#include <avr/eeprom.h>

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

#define MAX_OPTIONS			6
#define BTN_BOUNCE 			20

static void display_opt(unsigned char);
static void check_buttons(void);
static void show_time(void);
static void set_time(void);
static void set_date(void);
static void set_alarm(void);
static void check_alarm(void);
//static void act_alarm(void);

unsigned char item = 0, item_old = 0; //current menu position and earlier menu position
const uint8_t daysinmonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 }; //array with amount of days in every month
char day_of_week[7][15] = { "nie",
						   "pon",
						   "wto",
						   "sro",
						   "czw",
						   "pia",
						   "sob",
};

//array contains menu options
char menu_opt[MAX_OPTIONS][15] = { "Pokaz czas",
								   "Ustaw czas",
								   "Ustaw date",
								   "Ustaw alarm",
								   "Ustaw komunikat",
								   "Dzien dobry!"
};

uint8_t year = 0; //variable contains a current year
uint8_t month = 0; //variable contains a current month
uint8_t day = 0; //variable contains a current day
uint8_t hour = 0; //variable contains a current hour
uint8_t minute = 0; //variable contains a current minute
uint8_t second = 0; //variable contains a current second

uint8_t a1_hour=0;
uint8_t a1_minute=0;
uint8_t a1_second=0;

static int exit = 0;
static int choice = 0;

//main function
int main(void) {
	PORTD = 0xF0; //activate pull-up resistors on PORT D

	LCD_Initalize();
	LCD_Clear();
	LCD_Home();

	LCD_WriteText("> ");
	LCD_WriteText(&menu_opt[item][0]);
	LCD_GoTo(2,1);
	LCD_WriteText(&menu_opt[item+1][0]);

	while(1) {
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
	//previous button pressed in menu
	if(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){
		_delay_ms(BTN_BOUNCE);
		while(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){};
		item_old = item;
		item -= item < 1 ? 0 : 1;
		display_opt(item);
	}

	//next button pressed in menu
	if(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {
		_delay_ms(BTN_BOUNCE);
		while(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {};
		item_old = item;
		item += item > (MAX_OPTIONS - 2) ? 0 : 1;
		display_opt(item);
		_delay_ms(BTN_BOUNCE);
	}

	//select button pressed in menu
	if(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){
		_delay_ms(BTN_BOUNCE);
		while(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){};
		LCD_Clear();
		//if chosen show time option
		if (item == 0) {
			show_time();
		};
		//if chosen a set time option
		if (item == 1) {
			set_time();
		};
		//if chosen a set date option
		if (item == 2) {
			set_date();
		}
		if (item == 3) {
			set_alarm();
		}
		_delay_ms(BTN_BOUNCE);
	}

	//back button pressed in menu option
	if(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)){
		_delay_ms(BTN_BOUNCE);
		while(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)) {};
		if(item < (MAX_OPTIONS - 1)){
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

// function which shows current time
void show_time(void) {
	exit =0;
	while(exit == 0) {
		int dayy;
		ds1307_getdate(&year, &month, &day, &hour, &minute, &second);

		char current_date[15];
		char current_time[15];
		LCD_Clear();
		LCD_GoTo(2,0);

		sprintf(current_date, "%d/%d/%d", day, month, year);
		dayy = ds1307_getdayofweek(year, month, day);
		LCD_WriteText(&current_date[0]);
		LCD_WriteText("   ");
		LCD_WriteText(&day_of_week[dayy][0]);

		sprintf(current_time, "%d:%d:%d", hour, minute, second);
		LCD_GoTo(4,1);
		LCD_WriteText(&current_time[0]);

		_delay_ms(200);

		check_alarm();

		if(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)) {
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)) {};
			if(item < (MAX_OPTIONS - 1)) {
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
			exit = 1;
			_delay_ms(BTN_BOUNCE);
		}
	}
}

void set_time(void) {
	ds1307_getdate(&year, &month, &day, &hour, &minute, &second);

	exit =0;
	choice = 0;
	while(exit == 0) {
		char bufor[15];
		sprintf(bufor, "%d:%d:%d", hour, minute, second);
		LCD_Clear();
		LCD_GoTo(0,0);
		LCD_WriteText(&bufor[0]);
		if (choice == 0){
			LCD_GoTo(0,1);
			LCD_WriteText("Ustaw godzine");
		}
		if (choice == 1){
			LCD_GoTo(0,1);
			LCD_WriteText("Ustaw minute");
		}
		if (choice == 2){
			LCD_GoTo(0,1);
			LCD_WriteText("Ustaw sekunde");
		}

		if(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){};
			if (choice == 0) {
				if (hour > 1) {
					hour = hour - 1;
				}else hour = 0;
			}
			if (choice==1) {
				if (minute > 1) {
					minute = minute - 1;
				}else minute = 0;
			}
			if (choice==2) {
				if (second > 1) {
					second = second -1;
				}else second = 0;
			}
			_delay_ms(BTN_BOUNCE);
		}

		if(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {};
			if (choice==0) {
				if (hour < 23) {
				hour = hour + 1;
				}else hour = 1;
			}
			if (choice==1) {
				if (minute < 59) {
					minute = minute + 1;
				}else minute = 1;
			}
			if (choice==2) {
				if (second < 59) {
				second = second + 1;
				}else second = 1;
			}
			_delay_ms(BTN_BOUNCE);
		}

		if(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){};
			_delay_ms(BTN_BOUNCE);
			choice=choice+1;
			if (choice > 2) {
				choice=0;
			}
		}

		if(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)){
			exit=1;
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

void set_date(void) {
	ds1307_getdate(&year, &month, &day, &hour, &minute, &second);
	exit =0;
	choice = 0;
	while(exit==0) {
		char bufor[15];
		sprintf(bufor, "%d/%d/%d", day, month, year);
		LCD_Clear();
		LCD_GoTo(0,0);
		LCD_WriteText(&bufor[0]);
		if (choice==0){
			LCD_GoTo(0,1);
			LCD_WriteText("Ustaw dzien");
		}
		if (choice==1){
			LCD_GoTo(0,1);
			LCD_WriteText("Ustaw miesiac");
		}
		if (choice==2){
		LCD_GoTo(0,1);
		LCD_WriteText("Ustaw rok");
		}

		if(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)){};
			if (choice==2) {
				year = year - 1;
			}
			if (choice==1) {
				if (month > 1) {
				month = month - 1;
				}else month = 12;
			}
			if (choice==0) {
				if (day > 1){
				day = day -1;
				}else day = daysinmonth[month];
			}
			_delay_ms(BTN_BOUNCE);
		}

		if(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(NEXT_BUTTON_PIN, NEXT_BUTTON_BIT)) {};
			if (choice==2) {
				year = year + 1;
			}
			if (choice==1) {
				if (month < 12) {
					month = month + 1;
				}else month = 1;
			}
			if (choice==0) {
				if (day < daysinmonth[month]) {
				day = day + 1;
				}else day = 1;
			}
			_delay_ms(BTN_BOUNCE);
		}

		if(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){};;
			_delay_ms(BTN_BOUNCE);
			choice=choice+1;
			if (choice > 2) {
				choice=0;
			}
		}

		if(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)){
			exit=1;
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

void set_alarm(void) {
	//uint8_t a2_hour;
	//uint8_t a2_minute;
	//uint8_t a2_second;

	choice = 0;
	exit = 0;

	while(exit == 0) {
		char bufor[15];
		sprintf(bufor, "%d:%d", a1_hour, a1_minute);

		LCD_Clear();
		LCD_GoTo(0, 1);
		LCD_WriteText(&bufor[0]);

		if (choice == 0) {
			LCD_GoTo(0,0);
			LCD_WriteText("Ustaw godzine");
		}

		if (choice == 1) {
			LCD_GoTo(0,0);
			LCD_WriteText("Ustaw minute");
		}

		if(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)) {
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(PREV_BUTTON_PIN, PREV_BUTTON_BIT)) {};
			if (choice==0) {
				if (a1_hour < 23) {
					a1_hour = a1_hour + 1;
				}else a1_hour = 1;
			}
			if (choice==1) {
				if (a1_minute < 59) {
					a1_minute = a1_minute + 1;
				}else a1_minute = 1;
			}
			_delay_ms(BTN_BOUNCE);
		}

		if(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){
			_delay_ms(BTN_BOUNCE);
			while(bit_is_clear(SEL_BUTTON_PIN, SEL_BUTTON_BIT)){};;
			_delay_ms(BTN_BOUNCE);
			choice=choice+1;
			if (choice > 2) {
				choice=0;
			}
		}

		if(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)){
			exit=1;
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
}

void check_alarm(void) {
	if (hour == a1_hour && minute == a1_minute) {
		while(exit==0) {
			LCD_Clear();
			LCD_WriteText("ALARM 1");
			_delay_ms(200);
			if(bit_is_clear(BACK_BUTTON_PIN, BACK_BUTTON_BIT)){
				exit = 1;
			}
		}
	}
}
