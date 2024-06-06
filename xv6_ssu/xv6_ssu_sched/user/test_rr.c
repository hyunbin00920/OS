#include "types.h"
#include "stat.h"
#include "user.h"

#define INNER_LOOP 10000
#define OUTER_LOOP 20
#define PI 3.14 

int main(int argc, char **argv)
{
	int pid = getpid();
	float dummy = 0;
	int i,j;
	printf(1, "=== TEST START ===\n");
	pid = fork();
	if (pid == 0)	/* [P2] Child, its sched type must be ROUNDROBIN */
	{
		pid = fork();
		if (pid == 0) /* [P3] child-of-child, its sched type must be ROUNDROBIN */
		{
			for (i=0; i<OUTER_LOOP; i++)
			{
				for (j=0; j<INNER_LOOP; j++)
					dummy += PI*j;
				printf(1, "P3 (RR), i = %d, dummy = %x\n", i, dummy);
			}
		}
		else	/* [P2] Child */
		{
			for (i=0; i<OUTER_LOOP; i++)
			{
				for (j=0; j<INNER_LOOP; j++)
					dummy += PI*j;
				printf(1, "P2 (RR), i = %d, dummy = %x\n", i, dummy);
			}
			wait();
		}
	} 
	else	/* [P1] Parent, its sched type must be ROUNDROBIN */
	{
		for (i=0; i<OUTER_LOOP; i++)
		{
			for (j=0; j<INNER_LOOP; j++)
				dummy += PI*j;
			printf(1, "P1 (RR), i = %d, dummy = %x\n", i, dummy);
		}
		wait();
		printf(1, "=== TEST DONE ===\n");
	}
	exit();
}

