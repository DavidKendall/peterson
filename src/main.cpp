#include <stdbool.h>
#include <ucos_ii.h>
#include <mbed.h>
#include <display.h>

/*
*********************************************************************************************************
*                                            APPLICATION TASK PRIORITIES
*********************************************************************************************************
*/

typedef enum {
	APP_TASK_BUTTONS_PRIO = 4,
  APP_TASK_LED1_PRIO,
  APP_TASK_LED2_PRIO
} taskPriorities_t;

/*
*********************************************************************************************************
*                                            APPLICATION TASK STACKS
*********************************************************************************************************
*/

#define  APP_TASK_BUTTONS_STK_SIZE           256
#define  APP_TASK_LED1_STK_SIZE              256
#define  APP_TASK_LED2_STK_SIZE              256

static OS_STK appTaskButtonsStk[APP_TASK_BUTTONS_STK_SIZE];
static OS_STK appTaskLED1Stk[APP_TASK_LED1_STK_SIZE];
static OS_STK appTaskLED2Stk[APP_TASK_LED2_STK_SIZE];

/*
*********************************************************************************************************
*                                            APPLICATION FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void appTaskButtons(void *pdata);
static void appTaskLED1(void *pdata);
static void appTaskLED2(void *pdata);

/*
*********************************************************************************************************
*                                            GLOBAL TYPES AND VARIABLES 
*********************************************************************************************************
*/

typedef enum {
	JLEFT = 0,
	JRIGHT,
	JUP,
	JDOWN
} buttonId_t;

enum {
	FLASH_MIN_DELAY     = 1,
	FLASH_INITIAL_DELAY = 500,
	FLASH_MAX_DELAY     = 1000,
	FLASH_DELAY_STEP    = 50
};

static bool buttonPressedAndReleased(buttonId_t button);
static void incDelay(void);
static void decDelay(void);

static DigitalOut led1(P1_18);
static DigitalOut led2(P0_13);
static DigitalIn buttons[] = {P5_0, P5_4, P5_2, P5_1}; // LEFT, RIGHT, UP, DOWN
static AnalogIn potentiometer(P0_23);
static Display *d = Display::theDisplay();

static bool flashing = false;
static int32_t flashingDelay = FLASH_INITIAL_DELAY;

/*
*********************************************************************************************************
*                                            GLOBAL FUNCTION DEFINITIONS
*********************************************************************************************************
*/

int main() {

	/* Initialise the display */	
	d->fillScreen(WHITE);
	d->setTextColor(BLACK, WHITE);
  d->setCursor(2, 2);
	d->printf("EN0572 Lab 06");

  /* Initialise the OS */
  OSInit();                                                   

  /* Create the tasks */
  OSTaskCreate(appTaskButtons,                               
               (void *)0,
               (OS_STK *)&appTaskButtonsStk[APP_TASK_BUTTONS_STK_SIZE - 1],
               APP_TASK_BUTTONS_PRIO);
  
  OSTaskCreate(appTaskLED1,                               
               (void *)0,
               (OS_STK *)&appTaskLED1Stk[APP_TASK_LED1_STK_SIZE - 1],
               APP_TASK_LED1_PRIO);
  
  OSTaskCreate(appTaskLED2,                               
               (void *)0,
               (OS_STK *)&appTaskLED2Stk[APP_TASK_LED2_STK_SIZE - 1],
               APP_TASK_LED2_PRIO);

  
  /* Start the OS */
  OSStart();                                                  
  
  /* Should never arrive here */ 
  return 0;      
}

/*
*********************************************************************************************************
*                                            APPLICATION TASK DEFINITIONS
*********************************************************************************************************
*/

static void appTaskButtons(void *pdata) {
  /* Start the OS ticker -- must be done in the highest priority task */
  SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC);
  
  /* Task main loop */
  while (true) {
    if (buttonPressedAndReleased(JLEFT)) {
			flashing = false;
		}
		else if (buttonPressedAndReleased(JRIGHT)) {
			flashing = true;
		}
		else if (flashing && buttonPressedAndReleased(JUP)) {
			decDelay();
		}
		else if (flashing && buttonPressedAndReleased(JDOWN)) {
			incDelay();
		}
    OSTimeDlyHMSM(0,0,0,100);
  }
}

static void appTaskLED1(void *pdata) {
  while (true) {
		if (flashing) {
      led1 = !led1;
		}
    OSTimeDly(flashingDelay);
  }
}


static void appTaskLED2(void *pdata) {
  while (true) {
		if (flashing) {
      led2 = !led2;
		}
    OSTimeDly(flashingDelay);
  } 
}

/*
 * @brief buttonPressedAndReleased(button) tests to see if the button has
 *        been pressed then released.
 *        
 * @param button - the name of the button
 * @result - true if button pressed then released, otherwise false
 *
 * If the value of the button's pin is 0 then the button is being pressed,
 * just remember this in savedState.
 * If the value of the button's pin is 1 then the button is released, so
 * if the savedState of the button is 0, then the result is true, otherwise
 * the result is false.
 */
bool buttonPressedAndReleased(buttonId_t b) {
	bool result = false;
	uint32_t state;
	static uint32_t savedState[4] = {1,1,1,1};
	
	state = buttons[b].read();
  if ((savedState[b] == 0) && (state == 1)) {
		result = true;
	}
	savedState[b] = state;
	return result;
}

void incDelay(void) {
	if (flashingDelay + FLASH_DELAY_STEP > FLASH_MAX_DELAY) {
		flashingDelay = FLASH_MAX_DELAY;
	}
	else {
		flashingDelay += FLASH_DELAY_STEP;
	}
}

void decDelay(void) {
	if (flashingDelay - FLASH_DELAY_STEP < FLASH_MIN_DELAY) {
		flashingDelay = FLASH_MIN_DELAY;
	}
	else {
		flashingDelay -= FLASH_DELAY_STEP;
	}
}
	
