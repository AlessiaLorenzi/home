
/*
 * Global widget declarations.
 *        - EXTERNAL is set to extern if the
 *          defs file is not included from the
 *          main file.
 */


#ifdef DECLARE_BX_GLOBALS
#define EXTERNAL
XtIntervalId intervallo;
XtIntervalId save_interval;
#else
#define EXTERNAL extern
#endif

#define SPENTO 0
#define ACCESO 1


EXTERNAL  XtIntervalId  le_interval;
EXTERNAL  XtIntervalId  display_interval;
EXTERNAL  XtIntervalId  oscill_interval;
EXTERNAL  XtIntervalId  auto_interval;
EXTERNAL  XtIntervalId  intervallo;
EXTERNAL  XtIntervalId  save_interval;


EXTERNAL int      sinot_verbose;
EXTERNAL int      debug_verbose;
EXTERNAL int      calibr_verbose;
EXTERNAL int      open_verbose;
EXTERNAL int      loop_verbose;
EXTERNAL int      field_verbose;
EXTERNAL int      power_verbose;
EXTERNAL int      attenuation_verbose;
EXTERNAL int      a_gain_verbose;
EXTERNAL int      f_gain_verbose;
EXTERNAL int      net_verbose;
EXTERNAL int      set_verbose;
EXTERNAL int      bit_verbose;
EXTERNAL int      auto_verbose;
EXTERNAL int      soft_verbose;
EXTERNAL int      chopper_verbose;
EXTERNAL int      ls_verbose;
EXTERNAL int      config_verbose;
EXTERNAL int      phase_verbose;
EXTERNAL char     nome_dispositivo[20][100];
EXTERNAL char     tipo_dispositivo[20][100];
EXTERNAL char     nome_file_config[20][100];
EXTERNAL char     device_label[20][100];
EXTERNAL char     etichetta[20][100];
EXTERNAL int      tipo;
EXTERNAL int      selezionato;
EXTERNAL int      sottop_selez;
EXTERNAL int      prec_tipo;
EXTERNAL int      prec_selezionato;
EXTERNAL int      prec_sottop_selez;
EXTERNAL int      sub_page_index[20];
EXTERNAL int      sottop_tipo[20];
EXTERNAL int      max_sp;
EXTERNAL int      max_pages;
EXTERNAL int      next_page;
EXTERNAL int      pdr;

EXTERNAL float    field[20];
EXTERNAL int      loop[20];
EXTERNAL int      pha_loop;
EXTERNAL int      power[20];
EXTERNAL int      attenuation[20];
EXTERNAL int      a_gain[20];
EXTERNAL int      f_gain[20];
EXTERNAL int      pha_f_gain;
EXTERNAL float    kappa[20];
EXTERNAL float    max_field[20];
EXTERNAL float    min_field[20];
EXTERNAL float    atten_field[20];
EXTERNAL char     nome_calibr[20][100];
EXTERNAL float 	  ascisse[300][20];
EXTERNAL int      ordinate[300][20];
EXTERNAL int      lp_onoff[20];
EXTERNAL int      af_onoff[20];
EXTERNAL int      pf_onoff[20];
EXTERNAL int      amplifier_onoff[20];
EXTERNAL char     rf_tty[20][100];
EXTERNAL char     rf_host[20][100];
EXTERNAL int      rf_socket[20];
EXTERNAL int      rf_fildes[20];
EXTERNAL int      za_ampli_host[20][100];/*host su cui gira server porta parall per amplif. */
EXTERNAL int      za_ampli_socket[20];	/*socket su cui ascolta server porta parall per amplif. */
EXTERNAL int      za_ampli_fildes[20];	/* descrittore canale per comunicaz. con server porta parall per amplif. */
EXTERNAL char     za_tty[20][100];
EXTERNAL char     za_host[20][100];	/* host su cui gira server motori per tuner */
EXTERNAL int      za_socket[20];	/* socket server motori per tuner */
EXTERNAL int      za_fildes[20];	/* descrittore canale per comunicazione con server motori per tuner */
EXTERNAL int      za_par_io_port[20];	/* porta parallela per bit singoli */
EXTERNAL int      za_ampl_bit[20];	/* bit dell'amplificatore */
EXTERNAL int      motor[20];		/* numero del motore per il tuning */
EXTERNAL int      tun_fast[20];         /* numero di passi tuner fast */
EXTERNAL int      tun_slow[20];		/* numero di passi tuner slow */
EXTERNAL int      displ_errors_onoff;	/* abilitazione letture arrori */
EXTERNAL int      chopper_m_d_port;	/* chopper multiplate delay port */
EXTERNAL int      chopper_p_w_port;	/* chopper width port */
EXTERNAL int      chopper_d_c_port;	/* chopper duty cycle port */
EXTERNAL int      chopper_fp_sp_port;	/* chopper fast pulsing slow pulsing port */
EXTERNAL int      chopper_par_io_port;	/* chopper parallel io port */
EXTERNAL int      chopper_phase_port;	/* chopper phase port */
EXTERNAL int      chopper_onoff_bit;	/* chopper on off bit */
EXTERNAL int      chopper_intext_bit;	/* chopper riferimento int/ext bit */
EXTERNAL int      chopper_spfp_bit;	/* chopper slow fast selector bit */
EXTERNAL int      chopper_socket;
EXTERNAL char     chopper_host[100];     
EXTERNAL int      chopper_fildes;
EXTERNAL int      chopper_onoff;
EXTERNAL int      chopper_intext;
EXTERNAL int      chopper_denom;
EXTERNAL int      chopper_m_d;
EXTERNAL int      chopper_p_w;
EXTERNAL int      chopper_d_c;
EXTERNAL int      chopper_phase;
EXTERNAL char     tubi_host[20][100];
EXTERNAL int      tubi_socket[20];
EXTERNAL int      tubi_par_io_port[20];
EXTERNAL int      tubi_bit[20];
EXTERNAL int      tubi[20];
EXTERNAL int      buncher_ph_port[20];
EXTERNAL int      buncher_ph[20];
EXTERNAL int      postchopper_ph_port[20];
EXTERNAL int      postchopper_ph[20];
EXTERNAL int      auto_oscill_onoff;
EXTERNAL int      ricerca_picco_onoff;
EXTERNAL int      loop_counter;
EXTERNAL int      a_loop_ph;
EXTERNAL char     file_da_leggere[100];
EXTERNAL char     file_da_scrivere[100];
EXTERNAL int      rf_is_alive[20];
EXTERNAL int      tu_is_alive[20];
EXTERNAL int      ampli_is_alive[20];
EXTERNAL int      chopper_is_alive;
EXTERNAL int	  to_do;
EXTERNAL int      next_to_do;
EXTERNAL int	  amplitude_error;
EXTERNAL int	  frequency_error;
EXTERNAL int	  new_amplitude_error;
EXTERNAL int	  new_frequency_error;
EXTERNAL int	  old_amplitude_error;
EXTERNAL int	  old_frequency_error;
EXTERNAL int      ampl_err[100];
EXTERNAL int      phase_step;
EXTERNAL int      phase_start;
EXTERNAL int      phase_stop;
EXTERNAL int      ph_max_index;
EXTERNAL int      ph_max_index1;
EXTERNAL int      ph_index;
EXTERNAL int      loop_phase[100];
EXTERNAL int      min_ampl_err;
EXTERNAL int      loop_for_min;
EXTERNAL int      oscill_is_on;
EXTERNAL int      index_for_min;
EXTERNAL int      index_for_min1;
EXTERNAL int      secondo_giro;
EXTERNAL int      auto_tuning_onoff;
EXTERNAL int      tu_step_fast_fast[20];
EXTERNAL int      tu_step_fast[20];
EXTERNAL int      tu_step_slow[20];
EXTERNAL float      end_fr_corr[20];
EXTERNAL int      tu_direction;
EXTERNAL int      ha_diminuito;
EXTERNAL int      ha_aumentato;
EXTERNAL int      tu_fa_sl;
EXTERNAL char      primo_messaggio[20][100];
EXTERNAL char      tu_primo_messaggio[20][100];
EXTERNAL char      ampli_primo_messaggio[20][100];
EXTERNAL int      answer_str[100];
EXTERNAL int      ok_1;
EXTERNAL int      ok_2;
EXTERNAL int      steps;
EXTERNAL int      step[20];
EXTERNAL float	  deltae;
EXTERNAL int      min_from_sat;
EXTERNAL int      auto_zeroampl_onoff;
EXTERNAL int      chopper_fast;
EXTERNAL int      chopper_slow;
EXTERNAL int      general_phase;
EXTERNAL int      general_phase_port;
EXTERNAL int      phase_socket;
EXTERNAL int      phase_fildes;
EXTERNAL char     phase_host[100];
EXTERNAL char     phase_primo_messaggio[100];
EXTERNAL int      phase_is_alive;
EXTERNAL int      phase_tuning_socket;
EXTERNAL int      phase_tuning_fildes;
EXTERNAL char     phase_tuning_host[100];
EXTERNAL char     phase_tuning_primo_messaggio[100];
EXTERNAL int      phase_tuning_is_alive;
EXTERNAL int      phase_motor;
EXTERNAL float   table[4096];
EXTERNAL double   mass_A;
EXTERNAL double   charge_Q;
EXTERNAL double   energy_E;
EXTERNAL char     load_save_action[100];
EXTERNAL char 	  error_message[4][100];
EXTERNAL int      ch_window_onoff;
EXTERNAL int      zero_f_onoff[20];
EXTERNAL int      zero_a_onoff[20];
EXTERNAL int      e1;
EXTERNAL int      error_amp1[20];
EXTERNAL int      error_amp2[20];
EXTERNAL int      auto_zero_onoff;
EXTERNAL int      zero_a_soglia[20];
EXTERNAL int      zero_a_step[20];
EXTERNAL int      zero_f_soglia[20];
EXTERNAL int      zero_f_step[20];
EXTERNAL int      da_corr[20];
EXTERNAL float    v_centro_new[20];
EXTERNAL float    v_centro_old[20];
EXTERNAL float    v_delta_new[20];
EXTERNAL float    v_delta_old[20];
EXTERNAL int      padre[20];
EXTERNAL int      sequenza[80];
EXTERNAL int      no_stop;
EXTERNAL int      seq_counter;

EXTERNAL int massa, carica, ampl, ph_corr;
EXTERNAL double beta, ti, corr, energia;

EXTERNAL int 	ph_refresh_onoff;
EXTERNAL int 	phase_loop;
EXTERNAL int 	ampl_correction;
EXTERNAL int 	ph_lock_onoff;
EXTERNAL int 	ph_stab_onoff;
EXTERNAL int 	ref_intext;

EXTERNAL float 	new_field1;
EXTERNAL float 	new_field2;
EXTERNAL int 	altro_giro;
EXTERNAL int 	altro_giro1;
EXTERNAL int 	altro_giro2;
EXTERNAL int 	err_ampiezza;
EXTERNAL int 	tutto_bene;
EXTERNAL int 	val_qp[20];
EXTERNAL int 	amp_err_min[20];
EXTERNAL float 	max_var_field[20];
EXTERNAL int 	w_index[20];
EXTERNAL int 	is_the_second[20];
EXTERNAL int 	ampl_color;
EXTERNAL int 	phase_color;
EXTERNAL int 	ampl_color_sp;
EXTERNAL int 	phase_color_sp;
EXTERNAL char     ph_objects[20][100];
EXTERNAL char     selected_phase[100];
EXTERNAL int     selected_phase_position;
EXTERNAL int     le_port;
EXTERNAL int     le_socket;
EXTERNAL int     le_fildes;
EXTERNAL int     le_is_alive;
EXTERNAL int     le_f_gain;
EXTERNAL char     le_host[100];
EXTERNAL char     le_primo_messaggio[100];
EXTERNAL int	le_readings_onoff;
EXTERNAL int	le_offset;
EXTERNAL int	le_ph_error;
