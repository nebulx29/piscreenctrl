/***
 * Program: piscreenctrl
 * Usage: piscreenctrl 
 * Description: run as daemon, listening on GPIO pin (wiringPi numbering)
 *              on high signal the command is executed, Ctrl+C to stop program
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <pthread.h>

#define LOOP_DELAY 2000

// CONSTANTS
#define DEBUG   0
#define TIMESTAMP_LENGTH 27
#define PIR_PIN 1
#define SIGNAL  1
#define DISPLAY_OFF 0
#define DISPLAY_ON  1
#define DEFAULT_TIMER_SEC 180

const char *CMDS[] = {
		"/home/pi/bin/display-off.sh",
		"/home/pi/bin/display-on.sh"
	};

// GLOBAL VARIABLES
char timestamp[TIMESTAMP_LENGTH];
int timer = DEFAULT_TIMER_SEC;

// DECLARATIONS
void printUsage(void);
int init(void);
void exInt0_ISR(void);
void display_off(void);
void display_on(void);

// DEFINITIONS
void printUsage() {
	fprintf(stderr, "Usage: piscreenctrl\n"
                "   Program listens on GPIO [PIR_PIN] for HIGH signal and executes turns on/off display.\n"
		);
	exit(EXIT_FAILURE);
}

/*
 * initialization routine
 */ 
int init(void) {
	if (wiringPiSetup()<0) {
		fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
		return 1;
	}
	wiringPiISR(PIR_PIN, SIGNAL, &exInt0_ISR);
	if (DEBUG) {
		createTimestamp(timestamp);
		fprintf(stdout,"[%s] [piscreenctrl] init() pir=%d signal=HIGH)\n", timestamp, PIR_PIN);
		fflush(stdout);
	}
	return 0;
}

/* 
 * countdown runs in own thread, counts timer down by 1 per sec
 */ 
void *countdown(void *threadid) {
    long tid;
    tid = (long)threadid;
    if (DEBUG) {
		createTimestamp(timestamp);
		fprintf(stdout,"[%s] [piscreenctrl] countdown thread started threadid=%d\n", timestamp, tid);
	}
	while (1) {
		delay(1000); // 1000ms
		timer = timer - 1;
		timer = timer<0 ? 0 : timer;
	}
    pthread_exit(NULL);
 }
 
 void display_off(void) {
		createTimestamp(timestamp);
		fprintf(stdout, "[%s] display_off()\n", timestamp);
		fflush(stdout);
		system(CMDS[DISPLAY_OFF]);
 }

 void display_on(void) {
		createTimestamp(timestamp);
		fprintf(stdout, "[%s] display_on()\n", timestamp);
		fflush(stdout);
		system(CMDS[DISPLAY_ON]);
 }


/* 
 * GPIO interrupt on PIR motion sensor rising to high
 */ 
void exInt0_ISR(void) {
	if (DEBUG) {
		createTimestamp(timestamp);
		fprintf(stdout, "[%s] exInt0_ISR(%d): Motion detected. Reseting timer to %dsec\n", timestamp, PIR_PIN, DEFAULT_TIMER_SEC);
		fflush(stdout);
	}
	timer = DEFAULT_TIMER_SEC;
	return;
}

int createTimestamp(char* buffer) {
  int millisec;
  struct tm* tm_info;
  struct timeval tv;
  char buffer2[TIMESTAMP_LENGTH];
  
  gettimeofday(&tv, NULL);

  millisec = lrint(tv.tv_usec/1000.0); 
  if (millisec>=1000) { 
    millisec -=1000;
    tv.tv_sec++;
  }

  tm_info = localtime(&tv.tv_sec);
  strftime(buffer, TIMESTAMP_LENGTH, "%Y-%m-%d %H:%M:%S", tm_info); 
  sprintf(buffer, "%s.%d", buffer, millisec);
}

int main(int argc, char* argv[]) {
	if (argc!=1) {
		printUsage();
		exit(1);
		return 1;
	}
	
	init();
	
	pthread_t *thread;
	long threadid;
	int rc;
	rc = pthread_create(&thread, NULL, countdown, (void *)threadid);
	
	if (rc){
	  fprintf(stderr,"ERROR; return code from pthread_create() is %d\n", rc);
	  return(1);
	  exit(1);
	}	
	
	while (1) {
		delay(LOOP_DELAY);
		if (DEBUG) fprintf(stdout, "%d ",timer);
		if (DEBUG) fflush(stdout);
		
		if (timer==0) {
			display_off();
			while (!timer) {
				delay(1000);
			}
			display_on();
		}
	}
	
	exit(EXIT_SUCCESS);
	return 0;
}



