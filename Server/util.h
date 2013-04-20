/*
 * useful structures/macros
 *
 * 2011, Operating Systems
 */


#include <stdio.h>
#include <stdlib.h>

/* error printing macro */
#define ERR(call_description)				\
	do {						\
		fprintf(stderr, "(%s, %d): ",		\
				__FILE__, __LINE__);	\
			perror(call_description);	\
	} while (0)

/* print error (call ERR) and exit */
#define DIE(assertion, call_description)		\
	do {						\
		if (assertion) {			\
			ERR(call_description);		\
			exit(EXIT_FAILURE);		\
		}					\
	} while(0)
