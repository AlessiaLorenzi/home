void maprint_data(string)
char *string;
{

char comando_di_stampa[90], form1[90], form2[90], form3[90];
int cnt;
FILE *file_to_print;
FILE *command_file;

printf(" * * 1 * * \n");

command_file = fopen("print_commands","r");

fgets( comando_di_stampa, 90, command_file );

fgets( form1, 90, command_file );
fgets( form2, 90, command_file );
fgets( form3, 90, command_file );
printf("  >>>>>>> %s\n%s\n%s\n", form1, form2, form3 );

fclose(command_file);

printf(" * * 2 * * \n");

pr_maread_data(string);

printf(" * * 3 * * \n");

system("date > file_to_print");

file_to_print = fopen("file_to_print","a");
fprintf(file_to_print, "\n");

printf(" * * 4 * * \n");
fprintf(file_to_print,"MAGNETS :\n\n");

for (cnt = 1; cnt<numero_bottoni+1 ; cnt++) /* SOSTITUITO */
    {
    if (pr_supply[magnet[cnt].alim1].mode == 1 &&
        pr_supply[magnet[cnt].alim1].on_off == 1)
        fprintf(file_to_print,form1, 
            magnet[cnt].n_alim1, pr_supply[magnet[cnt].alim1].current,
            pr_supply[magnet[cnt].alim1].wa_current/10000.0);
                    
    if (pr_supply[magnet[cnt].alim1].mode == 1 &&
        pr_supply[magnet[cnt].alim1].on_off == 0)
        fprintf(file_to_print,form2, 
                magnet[cnt].n_alim1, pr_supply[magnet[cnt].alim1].current,
            pr_supply[magnet[cnt].alim1].wa_current/10000.0);

    if (pr_supply[magnet[cnt].alim1].mode == 0)
        fprintf(file_to_print, form3, magnet[cnt].n_alim1);
                        

    if ( ( magnet[cnt].alim2 != 9999 ) && ( magnet[cnt].magnet != 999999 ) )
        {
        if (pr_supply[magnet[cnt].alim2].mode == 1 &&
            pr_supply[magnet[cnt].alim2].on_off == 1)
            fprintf(file_to_print,form1, 
                    magnet[cnt].n_alim2, pr_supply[magnet[cnt].alim2].current,
                    pr_supply[magnet[cnt].alim2].wa_current/10000.0);
                                
        if (pr_supply[magnet[cnt].alim2].mode == 1 &&
            pr_supply[magnet[cnt].alim2].on_off == 0)
            fprintf(file_to_print,form2, 
                    magnet[cnt].n_alim2, pr_supply[magnet[cnt].alim2].current,
                    pr_supply[magnet[cnt].alim2].wa_current/10000.0);

        if (pr_supply[magnet[cnt].alim2].mode == 0)
            fprintf(file_to_print,form3, magnet[cnt].n_alim2);
                            
        }
    }

printf(" * * 5 * * \n");

fprintf(file_to_print,"\n\nSTEERERS :\n\n");

for ( cnt = 1; cnt < numero_bottoni + 1; cnt++ )
        
    {

    if ( button[cnt].type == 2 )
        {
        if ( button[cnt].obj1 != 9999 &&
	     button[cnt].obj1 != 999999 )
            {
            if ( pr_steerer_supply[steerer[button[cnt].obj1].alim1].mode == 0 )
                fprintf(file_to_print, "%-10s  STUB\n", 
                        steerer[button[cnt].obj1].name);

            else
                fprintf(file_to_print, "%-10s  %-5.2f\n", 
                    steerer[button[cnt].obj1].name,
                    pr_steerer_supply[steerer[button[cnt].obj1].alim1].current);


            if ( pr_steerer_supply[steerer[button[cnt].obj1].alim2].mode == 0 )
                fprintf(file_to_print, "            STUB\n");


            else
                fprintf(file_to_print, "            %-5.2f\n", 
                    pr_steerer_supply[steerer[button[cnt].obj1].alim2].current);

 
            }
    
        if ( button[cnt].obj3 != 9999 && 
	     button[cnt].obj3 != 999999 )
            {
            if ( pr_steerer_supply[steerer[button[cnt].obj3].alim1].mode == 0 )
                fprintf(file_to_print, "%-10s  STUB\n", 
                        steerer[button[cnt].obj3].name);
            
            else
                fprintf(file_to_print, "%-10s  %-5.2f\n", 
                    steerer[button[cnt].obj3].name,
                    pr_steerer_supply[steerer[button[cnt].obj3].alim1].current);

            if ( pr_steerer_supply[steerer[button[cnt].obj3].alim2].mode == 0 )
                fprintf(file_to_print, "            STUB\n");


            else
                fprintf(file_to_print, "            %-5.2f\n", 
                    pr_steerer_supply[steerer[button[cnt].obj3].alim2].current);



            }


        }

    }        


printf(" * * 6 * * \n");

fprintf(file_to_print,"\ndati stampati dal file %s", string);

fclose(file_to_print);

printf(" * * 7 * * \n");

printf(" * * 7.5 * * %s\n",comando_di_stampa );

system(comando_di_stampa);

printf(" * * 8 * * \n");

}
