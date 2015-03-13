
/****************************************************************************** 
 * 
 * Description: Slavedriver.c	
 * 		Pid controller for worker threads.  Spins up new workers and
 * 		keeps tabs on them.  Passes work to the workers and keeps track
 * 		of overall worker health.  Requires a pid of it's own aside 
 * 		from master.
 *
 * Version:	1.0
 * Revision: 	1.0
 * Created:	2015-03-13 Fri 11:20 AM
 * Compiler: 	clang --std=c11
 *
 * Author:	michael.osiecki
 * Org: 	kohls
 *
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"
#include "redisconnector.h"
#include "slavedriver.h"

#define DEBUG 1
#define PIDCOUNT 10
// Every 100 cycles do a health check of workers
#define HEALTHCOUNT 100


int init_slavedriver() {
#ifdef DEBUG
	log_info("Initializing slavedriver pid");
#endif
	pid_t slavedriver = fork(); 
	if (slavedriver == -1) {
		log_err("Error forking slavedriver!");
		exit(EXIT_FAILURE);
	} else if (slavedriver == 0) {
#ifdef DEBUG
		log_info("We're the slavedriver, %lu!", (long)getpid());
#endif
	} else {
#ifdef DEBUG
		log_info("We're the master, %lu!  Returning...", (long)getppid());
#endif
		return 1;
	}

	// Lets spin up some workers
	for (int i=0; i <= PIDCOUNT; i++) {
	}

#ifdef DEBUG
	log_info("Starting slavedriver loop");
#endif
	int cycles = 0;
	while (1) {
		cycles++;
		/* Slavedriver needs to do the following things: 
		 * 1 - Spin up workers and put them in a wait for socket mesg state
		 * 2 - Retrieve worker health at regular intervals
		 *
		 * Workers will poll redis for work
		 */

		log_info("Cycle complete, sleeping a bit...");

		sleep(3);
		// TESTING only loop 5 times
		if (cycles >= 5) 
			break;
	}
#ifdef DEBUG
	log_info("Slavedriver is ready to die");
#endif
	close_redis();
	// Slavedriver is ready to die
	// Exit instead of return so we kill the pid
#ifdef DEBUG
	log_info("Slavedriver is dead!");
#endif
	exit(1);
}

