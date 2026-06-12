#define REMOTE 1
#define LOCAL 0

struct line		{
			char name[100];
			int mode;
			int verbose;
			int alim[12];
			FILE *rs232r;
			FILE *rs232w;
			int fildes;
			} ;


struct alim_data	{
			int address;
			FILE *rs232r;
			FILE *rs232w;
			int mode;
			int fildes;
			int verbose;
			} ;
