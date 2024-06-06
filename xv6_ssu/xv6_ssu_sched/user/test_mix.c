#include "types.h"
#include "stat.h"
#include "user.h"

#define INNER_LOOP 5000000
#define OUTER_LOOP 20
#define PI 3.14

int main(int argc, char **argv)
{
	int pid = getpid();
    float dummy = 0;
    int i,j;
	printf(1, "=== TEST START ===\n");
    pid = fork();
    if (pid == 0) /* [P2] child, its sched type must be ROUNDROBIN */
    {
		for (i=0; i<3; i++)
        {
			for (j=0; j<INNER_LOOP; j++)
				dummy += PI*j;
            printf(1, "P2 (RR), i = %d, dummy = %x\n", i, dummy);
        }
        pid = fork();
        if (pid == 0) /* [P3] child-of-child, its sched type must be ROUNDROBIN */
        {
			dummy = 0;
			for (i=0; i<3; i++)
            {
                for (j=0; j<INNER_LOOP; j++)
                    dummy += PI*j;
				printf(1, "P3 (RR), i = %d, dummy = %x\n", i, dummy);
            }
			pid = fork_rt(20);
			if (pid == 0) /* [PA], its priority must be 20, sched type must be PRIORITY */
			{
				dummy = 0;
				printf(1, "=== RT SCHED START ===\n");
                for (i=0; i<3; i++)
                {
                    for (j=0; j<INNER_LOOP; j++)
                        dummy += PI*j;
                    printf(1, "PA (RT), i = %d, dummy = %x\n", i, dummy);
                }
                pid = fork_rt(10);
                if(pid == 0) /* [PB], its priority must be 10, sched type must be PRIORITY */
                {
                    for (i=0; i<3; i++)
                    {
                        for (j=0; j<INNER_LOOP; j++)
							dummy += PI*j;
                        printf(1, "PB (RT), i = %d, dummy = %x\n", i, dummy);
                    }
					pid = fork_rt(11);
					if(pid == 0) /* [PC], its priority must be 11, sched type must be PRIORITY*/
					{
						for (i=0; i<3; i++)
						{
							for (j=0; j<INNER_LOOP; j++)
								dummy += PI*j;
							printf(1, "PC (RT), i = %d, dummy = %x\n", i, dummy);
						}
						pid = fork();
						if(pid == 0) /* [P4], its sched type must be ROUNDROBIN */
						{
							for (i=0; i<6; i++)
                            {
                                for (j=0; j<INNER_LOOP; j++)
                                    dummy += PI*j;
                                printf(1, "P4 (RR), i = %d, dummy = %x\n", i, dummy);
                            }
						}
						else /* [PC] */
						{
							for (i=3; i<6; i++)
							{
								for (j=0; j<INNER_LOOP; j++)
									dummy += PI*j;
								printf(1, "PC (RT), i = %d, dummy = %x\n", i, dummy);
							}
							wait();
						}
					}
					else /* [PB] */
					{
						for (i=3; i<6; i++)
                        {
                            for (j=0; j<INNER_LOOP; j++)
                                dummy += PI*j;
                            printf(1, "PB (RT), i = %d, dummy = %x\n", i, dummy);
                        }
						wait();
					}
                }
                else /* [PA] */
                {
                    for (i=0; i<3; i++)
                    {
                        for (j=0; j<INNER_LOOP; j++)
                            dummy += PI*j;
                        printf(1, "PA (RT), i = %d, dummy = %x\n", i, dummy);
                    }
					printf(1, "=== RT SCHED DONE ===\n");
                    wait();
                }
			}
			else /* [P3] child-of-child */
			{
				for (i=3; i<6; i++)
                {
                    for (j=0; j<INNER_LOOP; j++)
						dummy += PI*j;
                    printf(1, "P3 (RR), i = %d, dummy = %x\n", i, dummy);
                }
				wait();
			}
        }
        else /* [P2] child */
        {
			for (i=3; i<6; i++)
            {
				for (j=0; j<INNER_LOOP; j++)
					dummy += PI*j;
				printf(1, "P2 (RR), i = %d, dummy = %x\n", i, dummy);
            }
			wait();
        }
    }
    else /* [P1] parent, its sched type must be ROUNDROBIN*/
    {
        for (i=0; i<6; i++)
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

