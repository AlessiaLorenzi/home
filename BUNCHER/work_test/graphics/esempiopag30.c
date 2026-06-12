#include <Xm/Xm.h>
#include <X11/Shell.h>
#include <Xm/Label.h>
void main( int argc, char *argv[])
{
	char *application_class ="Xdate";
	XmString x_the_date;
	XtAppContext context;
	Widget toplevel,date_label;
	Arg args[10];
	int num_args=0;
	XtSetArg(args[num_args],XmNallowShellResize,False);
	num_args++;
	toplevel=XtAppInitialize(&context,"Xdate",NULL,0,&argc,argv,NULL,args,num_args);
	x_the_date=XmStringCreateLtoR("teh_date",XmSTRING_DEFAULT_CHARSET);
	num_args=0;
	XtSetArg(args[num_args],XmNlabelString,x_the_date);
	num_args++;
	date_label=XmCreateLabel(toplevel,"date label",args,num_args);
	XtManageChild(date_label);
	XmStringFree(x_the_date);
	XtRealizeWidget(toplevel);
	XtAppMainLoop(context);
}

