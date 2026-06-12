
/* ma page widgets */

extern Widget ma_shell_window, mashell_box, macom_box,
       ma_lab, maexit_but, masave_but, maread_but, maprint_but,
       maalarm_but, maalarm_lab, mawarning_lab, mawarning_but,
       maremall_but, maresetall_but, mazecurall_but, maallon_but,
       fast_load_but, fast_save_but;


extern Widget ma_comm_shell, ma_setup_box;

extern Widget ma_refr_onoff_but, ma_magexit_but,
              ma_next_but, ma_previous_but ;

extern Widget ma_box_a, ma_box_b, ma_box_c,
       ma_name_label_a, ma_name_label_b, ma_name_label_c,
       ma_cur_lab_a, ma_cur_lab_b, ma_cur_lab_c,
       ma_curimp_name_lab_a, ma_curimp_name_lab_b, ma_curimp_name_lab_c,
       ma_curatt_name_lab_a, ma_curatt_name_lab_b, ma_curatt_name_lab_c,
       ma_curimp_lab_a, ma_curimp_lab_b, ma_curimp_lab_c,
       ma_curatt_lab_a, ma_curatt_lab_b, ma_curatt_lab_c,
       ma_curimp_perc_lab_a, ma_curimp_perc_lab_b, ma_curimp_perc_lab_c,
       ma_curatt_perc_lab_a, ma_curatt_perc_lab_b, ma_curatt_perc_lab_c,
       ma_curup_but_a, ma_curup_but_b, ma_curup_but_c,
       ma_curdwn_but_a, ma_curdwn_but_b, ma_curdwn_but_c,
       ma_curup_fast_but_a, ma_curup_fast_but_b, ma_curup_fast_but_c,
       ma_curdwn_fast_but_a, ma_curdwn_fast_but_b, ma_curdwn_fast_but_c,
       ma_cur_ruler_a, ma_cur_ruler_b, ma_cur_ruler_c,
       ma_v1_lab_a, ma_v1_lab_b, ma_v1_lab_c,
       ma_v2_lab_a, ma_v2_lab_b, ma_v2_lab_c,
       ma_v3_lab_a, ma_v3_lab_b, ma_v3_lab_c,
       ma_v1_text_lab_a, ma_v1_text_lab_b, ma_v1_text_lab_c,
       ma_v2_text_lab_a, ma_v2_text_lab_b, ma_v2_text_lab_c,
       ma_v3_text_lab_a, ma_v3_text_lab_b, ma_v3_text_lab_c,
       ma_volt_lab_a, ma_volt_lab_b, ma_volt_lab_c,
       ma_temp_lab_a, ma_temp_lab_b, ma_temp_lab_c,
       ma_trans_lab_a, ma_trans_lab_b, ma_trans_lab_c,
       ma_volt_text_lab_a, ma_volt_text_lab_b, ma_volt_text_lab_c,
       ma_volt_perc_lab_a, ma_volt_perc_lab_b, ma_volt_perc_lab_c,
       ma_temp_text_lab_a, ma_temp_text_lab_b, ma_temp_text_lab_c,
       ma_trans_text_lab_a, ma_trans_text_lab_b, ma_trans_text_lab_c,
       ma_onoff_but_a, ma_onoff_but_b, ma_onoff_but_c,
       ma_remloc_but_a, ma_remloc_but_b, ma_remloc_but_c,
       ma_reset_but_a, ma_reset_but_b, ma_reset_but_c,
       ma_status_lab_1_a, ma_status_lab_1_b, ma_status_lab_1_c,
       ma_status_lab_2_a, ma_status_lab_2_b, ma_status_lab_2_c,
       ma_status_lab_3_a, ma_status_lab_3_b, ma_status_lab_3_c,
       ma_status_lab_4_a, ma_status_lab_4_b, ma_status_lab_4_c,
       ma_status_lab_5_a, ma_status_lab_5_b, ma_status_lab_5_c,
       ma_status_lab_6_a, ma_status_lab_6_b, ma_status_lab_6_c,
       ma_status_lab_7_a, ma_status_lab_7_b, ma_status_lab_7_c,
       ma_status_lab_8_a, ma_status_lab_8_b, ma_status_lab_8_c,
       ma_status_lab_9_a, ma_status_lab_9_b, ma_status_lab_9_c,
       ma_status_lab_10_a, ma_status_lab_10_b, ma_status_lab_10_c,
       ma_status_lab_11_a, ma_status_lab_11_b, ma_status_lab_11_c,
       ma_status_lab_12_a, ma_status_lab_12_b, ma_status_lab_12_c,
       ma_status_lab_13_a, ma_status_lab_13_b, ma_status_lab_13_c,
       ma_status_lab_14_a, ma_status_lab_14_b, ma_status_lab_14_c,
       ma_status_lab_15_a, ma_status_lab_15_b, ma_status_lab_15_c,
       ma_status_lab_16_a, ma_status_lab_16_b, ma_status_lab_16_c;

/* STEFANIA : aggiunti 3 widget grip, uno per alimentatore */
extern Widget ma_grip_a,         ma_grip_b,         ma_grip_c,
              ma_incdec_a,       ma_incdec_b,       ma_incdec_c;

/* STEFANIA : aggiunti widget per il controllo unificato di 2Q e 3Q */
extern Widget ma2q3q_box, ma2q3q_label,
              ma2q3q_I_lab, ma2q3q_I_ruler,
              ma2q3q_I_griplab, ma2q3q_I_grip,
              ma2q3q_D_lab, ma2q3q_D_ruler,
              ma2q3q_D_griplab, ma2q3q_D_grip ;



/* pagina steerer */


extern Widget st_page_but;
extern int pagina_steerers;
extern Widget steerer_shell, st_setting_but  , scatoletta;




extern Widget	steerer_box_a, 	st_name_lab_a, st_onoff_but_a, st_display_lab_a,
		steerer_box_b, 	st_name_lab_b, st_onoff_but_b, st_display_lab_b;


extern Widget	st_incdec_a_1,
		st_incdec_a_2,
		st_incdec_b_1,
		st_incdec_b_2;

extern Widget	st_real_a_1,
		st_real_a_2,
		st_real_b_1,
		st_real_b_2;

extern Widget	st_grip_a_1, st_curdwn_fast_but_a_1, st_curdwn_but_a_1, st_cur_ruler_a_1, st_curup_but_a_1, st_curup_fast_but_a_1,
		st_grip_a_2, st_curdwn_fast_but_a_2, st_curdwn_but_a_2, st_cur_ruler_a_2, st_curup_but_a_2, st_curup_fast_but_a_2,
		st_grip_b_1, st_curdwn_fast_but_b_1, st_curdwn_but_b_1, st_cur_ruler_b_1, st_curup_but_b_1, st_curup_fast_but_b_1,
		st_grip_b_2, st_curdwn_fast_but_b_2, st_curdwn_but_b_2, st_cur_ruler_b_2, st_curup_but_b_2, st_curup_fast_but_b_2;







extern struct magnet_data magnet[100];
extern struct magnet_data pr_magnet[100];
extern struct button_data button[100];
extern struct supply_data supply[100];
extern struct supply_data pr_supply[100];
extern struct steerer_data steerer[100];
extern struct steerer_data pr_steerer[100];
extern struct steerer_supply_data steerer_supply[100];
extern struct steerer_supply_data pr_steerer_supply[100];

extern char ma_suppl_status[30][100];


extern int verbose ;
extern int server_channel ;
extern char to_server[BUF_SIZE], from_server[BUF_SIZE] ;

extern int page_type, bottone_premuto;
extern int numero_bottone;

/*** STEFANIA 2
extern int refresh_onoff, old_refresh_onoff,st_refresh_onoff; 
***/

extern int refresh_onoff, st_refresh_onoff; 
extern XtIntervalId interval_id, alarminterval_id, st_interval_id;
extern int pixel_black, pixel_red, pixel_grey, pixel_yellow, pixel_white,
pixel_green, pixel_orange, pixel_coral, pixel_blue,
pixel_lightsteelblue3, pixel_brown;

extern int ma_alarms_onoff;

extern int a_x_center, a_y_center, b_x_center, b_y_center;

int numero_bottoni;
extern float dimensione_cursore;
extern float posizione_cursore;
