



/* questo file contiene tutte le definizioni che poi verranno
riferite come externals*/


struct magnet_data
	{
	int     magnet;		/* numero del magnete */
	char name[100];		/* numero del magnete */

	int      alim1;		/* numero dell'alimentatore 1 */
	char   n_alim1[100];	/* nome  dell'alimentatore  1 */
	int    a_alim1;		/* indir. dell'alimentatore 1 */
	int volt_alim1;		/* tens.  dell'alimentatore 1 */
	int  cur_alim1;		/* corr.  dell'alimentatore 1 */

	int      alim2;		/* numero dell'alimentatore 2 */
	char   n_alim2[100];	/* nome  dell'alimentatore  2 */
	int    a_alim2;		/* indir. dell'alimentatore 2 */
	int volt_alim2;		/* tens.  dell'alimentatore 2 */
	int  cur_alim2;		/* corr.  dell'alimentatore 2 */

	};

struct steerer_data
	{
	int number;
	char name[100];
	
	int alim1;
	int a_alim1;

	int alim2;
	int a_alim2;
	};


struct steerer_server_data
	{
	int socket;
	int fildes;
	char host[100];
	char lines[8][100];
	int modes[8];
	int verbose[8];
	int alim[8][4];
	int alim_addr[8][4];
	};

struct server_data
	{
	int socket;
	int fildes;
	char host[100];
	char line[100];
	int type;
	int mode;
	int verbose;
	char ahost[100];
	int asocket;
	int alim[16];
	int alim_addr[16];
	};

struct alim_data
	{
	int fildes;
	int address;
	int mode;
	};
	

struct steerer_alim_data
	{
	int fildes;
	int address;
	int mode;
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
