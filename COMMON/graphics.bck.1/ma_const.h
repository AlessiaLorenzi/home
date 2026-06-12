
/* ma page widgets */

struct magnet_data
        {
        int     magnet;                /* numero del magnete */
        char name[100];                /* numero del magnete */

        int      alim1;                /* numero dell'alimentatore 1 */
        char   n_alim1[100];        /* nome  dell'alimentatore  1 */
        int    a_alim1;                /* indir. dell'alimentatore 1 */
        int volt_alim1;                /* tens.  dell'alimentatore 1 */
        int  cur_alim1;                /* corr.  dell'alimentatore 1 */

        int      alim2;                /* numero dell'alimentatore 2 */
        char   n_alim2[100];        /* nome  dell'alimentatore  2 */
        int    a_alim2;                /* indir. dell'alimentatore 2 */
        int volt_alim2;                /* tens.  dell'alimentatore 2 */
        int  cur_alim2;                /* corr.  dell'alimentatore 2 */

	float i_value;                 /* STEFANIA: (i1+i2)/2 */
	float d_value;                 /* STEFANIA: (i1-i2)/(i1+i2) */
        };


struct button_data
        {
        int button;
        int type;
        int obj1;
        int obj2;
        int obj3;
        int obj4;
        };

struct supply_data
        {
        float current;
        int wa_current; /* STEFANIA: valore inviato al dac [0-999999] */
        int ra_current; /* STEFANIA: valore  letto dal dac [0-999999] */
        int maxcurrent;
        int maxvoltage;
        int on_off;
        int rem_loc;
        int finestra;
	int mode;
	int changed;
        };

struct steerer_supply_data
	{
	float current;
	int on_off;
	int finestra;
	int x_oppure_y;
	int mode;
	int changed;
	};



struct steerer_data
	{
	int steerer;
	char name[100];
	int alim1;
	int alim2;
	};

