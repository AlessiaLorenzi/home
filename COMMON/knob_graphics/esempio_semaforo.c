struct sembuf semaphore_struct ;


/* structure used to clear the semaphore (red light) */
semaphore_struct.sem_num = 0 ;
semaphore_struct.sem_op = -1 ;
semaphore_struct.sem_flg = 0 ;
	

				semop_retval = semop( semaphore_id, &semaphore_struct, 1 ) ;
                		if( semop_retval == -1 )
                        	printf( "*1* error in semaphore operation\n" ) ;
                		else
                        		{

                			/* turn on the green light */
                			semop_retval = semctl( semaphore_id,  0, SETVAL, 1 ) ;
					}
