
#include "alpi.h"
#include "ma_const.h"
#include "ma_ext_vars.h"
#include "ma.bit"
#include "bottsu"
#include "bottgiu"
#include <Shell.h>
#include <Grip.h>


#define MA_WIDTH      1022
#define MA_HEIGHT      516


#define SCROLLTRANS "\
<BtnDown>: StartScroll(Forward) MoveThumb() NotifyScroll(Proportional)\n\
<BtnMotion>: MoveThumb() NotifyScroll(Proportional)\n\
<BtnUp>: MoveThumb() NotifyThumb() EndScroll()"

#define GRIP_TRANS "\
<EnterWindow>: GripAction(enter)\n\
<LeaveWindow>: GripAction(leave)\n\
<BtnDown>:     GripAction(press)\n\
<BtnUp>:       GripAction(release)\n\
<Key>Right:    GripAction(right)\n\
<Key>Left:     GripAction(left)\n\
<Key>Up:       GripAction(up)\n\
<Key>Down:     GripAction(down)"



fill_box(  alimentatore, 
           ma_box, 
           ma_name_label, 
           ma_cur_lab, 
           ma_curimp_name_lab,
           ma_curatt_name_lab,
           ma_curimp_lab, 
           ma_curatt_lab, 
           ma_curimp_perc_lab, 
           ma_curatt_perc_lab, 
           ma_curup_but, 
           ma_curdwn_but,
           ma_curup_fast_but,
           ma_curdwn_fast_but,
           ma_cur_ruler, 
           ma_volt_text_lab,
           ma_volt_perc_lab, 
           ma_temp_text_lab, 
           ma_trans_text_lab,
           ma_v1_text_lab,
           ma_v2_text_lab,
           ma_v3_text_lab,
           ma_volt_lab,
           ma_temp_lab, 
           ma_trans_lab, 
           ma_v1_lab,
           ma_v2_lab,
           ma_v3_lab,
           ma_onoff_but,
           ma_remloc_but,
           ma_reset_but,
           etichetta,
           ma_status_lab_1,
           ma_status_lab_2,
           ma_status_lab_3,
           ma_status_lab_4,
           ma_status_lab_5,
           ma_status_lab_6,
           ma_status_lab_7,
           ma_status_lab_8,
           ma_status_lab_9,
           ma_status_lab_10,
           ma_status_lab_11,
           ma_status_lab_12,
           ma_status_lab_13,
           ma_status_lab_14,
           ma_status_lab_15,
           ma_status_lab_16,
           ma_grip, ma_incdec) /* STEFANIA :
                                  aggiunto il widget grip e incdec */

        int alimentatore;
        Widget ma_box, *ma_name_label;
        Widget *ma_cur_lab, *ma_curimp_name_lab, *ma_curatt_name_lab,
                            *ma_curimp_lab, *ma_curatt_lab, 
                            *ma_curimp_perc_lab, *ma_curatt_perc_lab;
        Widget *ma_curup_but, *ma_curdwn_but,
               *ma_curup_fast_but, *ma_curdwn_fast_but, *ma_cur_ruler;
        Widget *ma_volt_text_lab, *ma_volt_perc_lab,
               *ma_temp_text_lab, *ma_trans_text_lab;
        Widget *ma_v1_text_lab, *ma_v2_text_lab, *ma_v3_text_lab;

        Widget *ma_volt_lab, *ma_temp_lab, *ma_trans_lab;
        Widget *ma_v1_lab, *ma_v2_lab, *ma_v3_lab;


        Widget *ma_onoff_but, *ma_remloc_but, *ma_reset_but;
        char *etichetta;
        Widget *ma_status_lab_1, *ma_status_lab_2,
               *ma_status_lab_3, *ma_status_lab_4,
               *ma_status_lab_5, *ma_status_lab_6,
               *ma_status_lab_7, *ma_status_lab_8, 
               *ma_status_lab_9, *ma_status_lab_10,
               *ma_status_lab_11, *ma_status_lab_12,
               *ma_status_lab_13, *ma_status_lab_14,
               *ma_status_lab_15, *ma_status_lab_16;
        Widget *ma_grip, *ma_incdec; /* STEFANIA */

        {
        Arg args[9] ;
        int counter;

        extern ma_curdwn_fast_act(), ma_curdwn_act(),
               ma_ruler_jump(), ma_ruler_scroll(),
               ma_curup_act(), 
               ma_curup_fast_act(), ma_onoff_act(),
               ma_remloc_act(), ma_reset_act(),
               ma_grip_act() ; /* STEFANIA */


        *ma_name_label = XtCreateWidget ( "ma_name_label",
                                           labelWidgetClass,
                                           ma_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNlabel, etichetta ) ;
        XtSetValues(*ma_name_label , args, 1 ) ;
        XtManageChild(*ma_name_label);


        *ma_cur_lab = XtCreateWidget ( "ma_cur_lab", labelWidgetClass,
                                       ma_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, *ma_name_label);
        XtSetValues(*ma_cur_lab , args, 1 ) ;
        XtManageChild(*ma_cur_lab);


        *ma_curimp_name_lab = XtCreateWidget ( "ma_curimp_name_lab",
                              labelWidgetClass, ma_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, *ma_name_label);
        XtSetArg( args[1], XtNfromHoriz, *ma_cur_lab);
        XtSetValues(*ma_curimp_name_lab , args, 2 ) ;
        XtManageChild(*ma_curimp_name_lab);




        *ma_curimp_lab = XtCreateWidget ( "ma_curimp_lab",
                                          labelWidgetClass,
                         ma_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNlabel, "      " ) ;
        XtSetArg( args[1], XtNfromVert, *ma_name_label);
        XtSetArg( args[2], XtNfromHoriz, *ma_curimp_name_lab);
        XtSetValues(*ma_curimp_lab , args, 3 ) ;
        XtManageChild(*ma_curimp_lab);


        *ma_curatt_name_lab = XtCreateWidget ( "ma_curatt_name_lab",
                              labelWidgetClass, ma_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNfromVert, *ma_curimp_lab);
        XtSetArg( args[1], XtNfromHoriz, *ma_cur_lab);
        XtSetValues(*ma_curatt_name_lab , args, 2 ) ;
        XtManageChild(*ma_curatt_name_lab);


        *ma_curatt_lab = XtCreateWidget ( "ma_curatt_lab",
                                          labelWidgetClass,
                                           ma_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNlabel, "      " ) ;
        XtSetArg( args[1], XtNfromVert, *ma_curimp_lab);
        XtSetArg( args[2], XtNfromHoriz, *ma_curatt_name_lab);
        XtSetValues(*ma_curatt_lab , args, 3 ) ;
        XtManageChild(*ma_curatt_lab);



        *ma_curimp_perc_lab = XtCreateWidget ( "ma_curimp_perc_lab",
                              labelWidgetClass, ma_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNlabel, "      " ) ;
        XtSetArg( args[1], XtNfromVert, *ma_name_label);
        XtSetArg( args[2], XtNfromHoriz, *ma_curimp_lab);
        XtSetValues(*ma_curimp_perc_lab , args, 3 ) ;
        XtManageChild(*ma_curimp_perc_lab);



        *ma_curatt_perc_lab = XtCreateWidget ( "ma_curatt_perc_lab",
                              labelWidgetClass, ma_box, NULL, 0 ) ;
        XtSetArg( args[0], XtNlabel, "      " ) ;
        XtSetArg( args[1], XtNfromVert, *ma_curimp_perc_lab);
        XtSetArg( args[2], XtNfromHoriz, *ma_curatt_lab);
        XtSetValues(*ma_curatt_perc_lab , args, 3 ) ;
        XtManageChild(*ma_curatt_perc_lab);




/***************          BOTTONE        ************************/
*ma_curdwn_fast_but = XtCreateWidget( "ma_curdwn_fast_but",
                                      commandWidgetClass,
                                      ma_box,NULL, 0);

XtAddCallback(*ma_curdwn_fast_but , XtNcallback,
              ma_curdwn_fast_act, alimentatore ) ;

XtSetArg(args[0], XtNfromVert, *ma_curatt_lab);
XtSetValues(*ma_curdwn_fast_but, args, 1);
XtManageChild(*ma_curdwn_fast_but);



/***************          BOTTONE        ************************/
*ma_curdwn_but = XtCreateWidget("ma_curdwn_but", commandWidgetClass,
                 ma_box,NULL, 0);

XtAddCallback( *ma_curdwn_but , XtNcallback,
               ma_curdwn_act, alimentatore ) ;

XtSetArg(args[0], XtNfromVert, *ma_curatt_lab);
XtSetArg(args[1], XtNfromHoriz, *ma_curdwn_fast_but);
XtSetArg(args[2], XtNbitmap,"bottsu");
XtSetValues(*ma_curdwn_but, args, 2);
XtManageChild(*ma_curdwn_but);


/*****************          BARRA           ************************/

XtSetArg(args[0], XtNlength,500);
XtSetArg(args[1], XtNthickness,30);
XtSetArg(args[2], XtNorientation,XtorientHorizontal);

*ma_cur_ruler = XtCreateWidget ( "ma_cur_ruler", scrollbarWidgetClass,
                                 ma_box, args, 3);

XtOverrideTranslations( *ma_cur_ruler,
                        XtParseTranslationTable(SCROLLTRANS));

XtAddCallback( *ma_cur_ruler , XtNjumpProc,
               ma_ruler_jump, alimentatore ) ;

XtAddCallback( *ma_cur_ruler, XtNscrollProc,
               ma_ruler_scroll,alimentatore ) ;

XtSetArg(args[0], XtNfromHoriz, *ma_curdwn_but);
XtSetArg(args[1], XtNfromVert, *ma_curatt_lab);
XtSetValues(*ma_cur_ruler,args,2);
XtManageChild(*ma_cur_ruler);



/*****************          BOTTONE        ************************/
*ma_curup_but = XtCreateWidget("ma_curup_but",
                commandWidgetClass, ma_box,NULL,0);

XtAddCallback( *ma_curup_but , XtNcallback,
	       ma_curup_act, alimentatore ) ;

XtSetArg(args[0], XtNfromVert, *ma_curatt_lab);
XtSetArg(args[1], XtNfromHoriz, *ma_cur_ruler);
/*XtSetArg(args[2], XtNbitmap,bottgiu);*/
XtSetValues(*ma_curup_but, args, 2);
XtManageChild(*ma_curup_but);



/*****************          BOTTONE        ************************/
*ma_curup_fast_but = XtCreateWidget("ma_curup_fast_but",
                     commandWidgetClass, ma_box,NULL,0);

XtAddCallback(*ma_curup_fast_but , XtNcallback,
              ma_curup_fast_act, alimentatore ) ;

XtSetArg(args[0], XtNfromVert, *ma_curatt_lab);
XtSetArg(args[1], XtNfromHoriz, *ma_curup_but);
XtSetValues(*ma_curup_fast_but, args, 2);
XtManageChild(*ma_curup_fast_but);





*ma_onoff_but = XtCreateWidget( "ma_onoff_but", commandWidgetClass,
                                ma_box,NULL,0);

XtAddCallback( *ma_onoff_but ,
	       XtNcallback, ma_onoff_act, alimentatore ) ;

XtSetArg(args[0], XtNfromHoriz, *ma_name_label);
XtSetValues(*ma_onoff_but, args, 1);
XtManageChild(*ma_onoff_but);



*ma_remloc_but = XtCreateWidget("ma_remloc_but", commandWidgetClass,
                 ma_box,NULL,0);

XtAddCallback( *ma_remloc_but , XtNcallback,
	       ma_remloc_act, alimentatore ) ;

XtSetArg(args[0], XtNfromHoriz, *ma_onoff_but);
XtSetValues(*ma_remloc_but, args, 1);
XtManageChild(*ma_remloc_but);



*ma_reset_but = XtCreateWidget("ma_reset_but", commandWidgetClass,
                ma_box,NULL,0);

XtAddCallback( *ma_reset_but , XtNcallback,
	       ma_reset_act, alimentatore ) ;

XtSetArg(args[0], XtNfromHoriz, *ma_remloc_but);
XtSetValues(*ma_reset_but, args, 1);
XtManageChild(*ma_reset_but);




*ma_volt_text_lab = XtCreateWidget ( "ma_volt_text_lab",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "1" ) ;
XtSetArg( args[0], XtNfromVert, *ma_curdwn_fast_but);
XtSetValues(*ma_volt_text_lab , args, 1 ) ;
XtManageChild(*ma_volt_text_lab);

*ma_temp_text_lab = XtCreateWidget ( "ma_temp_text_lab",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "2" ) ;
XtSetArg( args[1], XtNfromVert, *ma_curdwn_fast_but);
XtSetArg( args[0], XtNfromHoriz, *ma_volt_text_lab);
XtSetValues(*ma_temp_text_lab , args, 2 ) ;
XtManageChild(*ma_temp_text_lab);

*ma_trans_text_lab = XtCreateWidget ( "ma_trans_text_lab",
                     labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "3" ) ;
XtSetArg( args[1], XtNfromVert, *ma_curdwn_fast_but);
XtSetArg( args[0], XtNfromHoriz, *ma_temp_text_lab);
XtSetValues(*ma_trans_text_lab , args, 2 ) ;
XtManageChild(*ma_trans_text_lab);

*ma_v1_text_lab = XtCreateWidget ( "ma_v1_text_lab",
                  labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "4" ) ;
XtSetArg( args[1], XtNfromVert, *ma_curdwn_fast_but);
XtSetArg( args[0], XtNfromHoriz, *ma_trans_text_lab);
XtSetValues(*ma_v1_text_lab , args, 2 ) ;
XtManageChild(*ma_v1_text_lab);

*ma_v2_text_lab = XtCreateWidget ( "ma_v2_text_lab",
                  labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "5" ) ;
XtSetArg( args[1], XtNfromVert, *ma_curdwn_fast_but);
XtSetArg( args[0], XtNfromHoriz, *ma_v1_text_lab);
XtSetValues(*ma_v2_text_lab , args, 2 ) ;
XtManageChild(*ma_v2_text_lab);

*ma_v3_text_lab = XtCreateWidget ( "ma_v3_text_lab",
                  labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "6" ) ;
XtSetArg( args[1], XtNfromVert, *ma_curdwn_fast_but);
XtSetArg( args[0], XtNfromHoriz, *ma_v2_text_lab);
XtSetValues(*ma_v3_text_lab , args, 2 ) ;
XtManageChild(*ma_v3_text_lab);

*ma_volt_lab = XtCreateWidget ( "ma_volt_lab",
               labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[1], XtNlabel, "7" ) ;
XtSetArg( args[0], XtNfromVert, *ma_volt_text_lab);
XtSetValues(*ma_volt_lab , args, 1 ) ;
XtManageChild(*ma_volt_lab);

*ma_volt_perc_lab = XtCreateWidget ( "ma_volt_perc_lab",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_volt_lab);
XtSetArg( args[1], XtNfromVert, *ma_volt_text_lab);
XtSetValues(*ma_volt_perc_lab , args, 2 ) ;
XtManageChild(*ma_volt_perc_lab);


*ma_temp_lab = XtCreateWidget ( "ma_temp_lab",
               labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "8" ) ;
XtSetArg( args[1], XtNfromVert, *ma_volt_text_lab);
XtSetArg( args[0], XtNfromHoriz, *ma_volt_perc_lab);
XtSetValues(*ma_temp_lab , args, 2 ) ;
XtManageChild(*ma_temp_lab);

*ma_trans_lab = XtCreateWidget ( "ma_trans_lab",
                labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "9" ) ;
XtSetArg( args[1], XtNfromVert, *ma_volt_text_lab);
XtSetArg( args[0], XtNfromHoriz, *ma_temp_lab);
XtSetValues(*ma_trans_lab , args, 2 ) ;
XtManageChild(*ma_trans_lab);

*ma_v1_lab = XtCreateWidget ( "ma_v1_lab",
             labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "10" ) ;
XtSetArg( args[1], XtNfromVert, *ma_volt_text_lab);
XtSetArg( args[0], XtNfromHoriz, *ma_trans_lab);
XtSetValues(*ma_v1_lab , args, 2 ) ;
XtManageChild(*ma_v1_lab);

*ma_v2_lab = XtCreateWidget ( "ma_v2_lab",
             labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "11" ) ;
XtSetArg( args[1], XtNfromVert, *ma_volt_text_lab);
XtSetArg( args[0], XtNfromHoriz, *ma_v1_lab);
XtSetValues(*ma_v2_lab , args, 2 ) ;
XtManageChild(*ma_v2_lab);

*ma_v3_lab = XtCreateWidget ( "ma_v3_lab",
             labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "12" ) ;
XtSetArg( args[1], XtNfromVert, *ma_volt_text_lab);
XtSetArg( args[0], XtNfromHoriz, *ma_v2_lab);
XtSetValues(*ma_v3_lab , args, 2 ) ;
XtManageChild(*ma_v3_lab);






*ma_status_lab_1 = XtCreateWidget ( "ma_status_lab_1",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[1], XtNlabel, "1" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_reset_but);
XtSetValues(*ma_status_lab_1 , args, 1 ) ;
XtManageChild(*ma_status_lab_1);

*ma_status_lab_2 = XtCreateWidget ( "ma_status_lab_2",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[1], XtNlabel, "2" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_status_lab_1);
XtSetValues(*ma_status_lab_2 , args, 1 ) ;
XtManageChild(*ma_status_lab_2);

*ma_status_lab_3 = XtCreateWidget ( "ma_status_lab_3",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_reset_but);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_1);
XtSetValues(*ma_status_lab_3 , args, 2 ) ;
XtManageChild(*ma_status_lab_3);

*ma_status_lab_4 = XtCreateWidget ( "ma_status_lab_4",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_status_lab_3);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_1);
XtSetValues(*ma_status_lab_4 , args, 2 ) ;
XtManageChild(*ma_status_lab_4);

*ma_status_lab_5 = XtCreateWidget ( "ma_status_lab_5",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "5" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_reset_but);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_3);
XtSetValues(*ma_status_lab_5 , args, 2 ) ;
XtManageChild(*ma_status_lab_5);

*ma_status_lab_6 = XtCreateWidget ( "ma_status_lab_6",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "6" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_status_lab_5);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_3);
XtSetValues(*ma_status_lab_6 , args, 2 ) ;
XtManageChild(*ma_status_lab_6);

*ma_status_lab_7 = XtCreateWidget ( "ma_status_lab_7",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "7" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_reset_but);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_5);
XtSetValues(*ma_status_lab_7 , args, 2 ) ;
XtManageChild(*ma_status_lab_7);

*ma_status_lab_8 = XtCreateWidget ( "ma_status_lab_8",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "8" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_status_lab_7);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_5);
XtSetValues(*ma_status_lab_8 , args, 2 ) ;
XtManageChild(*ma_status_lab_8);

*ma_status_lab_9 = XtCreateWidget ( "ma_status_lab_9",
                   labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "9" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_reset_but);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_7);
XtSetValues(*ma_status_lab_9 , args, 2 ) ;
XtManageChild(*ma_status_lab_9);

*ma_status_lab_10 = XtCreateWidget ( "ma_status_lab_10",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "10" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_status_lab_5);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_7);
XtSetValues(*ma_status_lab_10 , args, 2 ) ;
XtManageChild(*ma_status_lab_10);

*ma_status_lab_11 = XtCreateWidget ( "ma_status_lab_11",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "11" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_reset_but);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_9);
XtSetValues(*ma_status_lab_11 , args, 2 ) ;
XtManageChild(*ma_status_lab_11);

*ma_status_lab_12 = XtCreateWidget ( "ma_status_lab_12",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "12" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_status_lab_7);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_9);
XtSetValues(*ma_status_lab_12 , args, 2 ) ;
XtManageChild(*ma_status_lab_12);

*ma_status_lab_13 = XtCreateWidget ( "ma_status_lab_13",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "13" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_reset_but);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_11);
XtSetValues(*ma_status_lab_13 , args, 2 ) ;
XtManageChild(*ma_status_lab_13);

*ma_status_lab_14 = XtCreateWidget ( "ma_status_lab_14",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "14" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_status_lab_5);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_11);
XtSetValues(*ma_status_lab_14 , args, 2 ) ;
XtManageChild(*ma_status_lab_14);

*ma_status_lab_15 = XtCreateWidget ( "ma_status_lab_15",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "15" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_reset_but);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_13);
XtSetValues(*ma_status_lab_15 , args, 2 ) ;
XtManageChild(*ma_status_lab_15);

*ma_status_lab_16 = XtCreateWidget ( "ma_status_lab_16",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg( args[2], XtNlabel, "16" ) ;
XtSetArg( args[0], XtNfromHoriz, *ma_status_lab_7);
XtSetArg( args[1], XtNfromVert, *ma_status_lab_13);
XtSetValues(*ma_status_lab_16 , args, 2 ) ;
XtManageChild(*ma_status_lab_16);

/* STEFANIA : la creazione della grip area inizia ... */

*ma_grip = XtCreateWidget ( "ma_grip",
                    gripWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg(args[0], XtNfromVert, *ma_curatt_lab);
XtSetValues(*ma_grip , args, 1 ) ;
XtManageChild(*ma_grip);

XtOverrideTranslations( *ma_grip, XtParseTranslationTable(GRIP_TRANS));
XtAddCallback(  *ma_grip, XtNcallback, ma_grip_act, alimentatore ) ;

*ma_incdec = XtCreateWidget ( "ma_incdec",
                    labelWidgetClass, ma_box, NULL, 0 ) ;
XtSetArg(args[0], XtNfromVert, *ma_cur_lab);
XtSetValues(*ma_incdec , args, 1 ) ;
XtManageChild(*ma_incdec);

/* STEFANIA : la creazione della grip area e' finita */

}
