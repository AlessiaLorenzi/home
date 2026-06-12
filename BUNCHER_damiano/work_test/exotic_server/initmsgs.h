/* first message format, for alarm machine, socket info and mode
   mode = 0 means stub-mode, mode > 0 means normal mode
   ex: "console 4001 1" or "NONE 0 0" */
#define MSG1_FORMAT " %s %d %d "

/* motor second message format, for verbose option and tty name
   cryo number
   ex: "-v /dev/tty17 0"  or "-n /dev/tty17 0" */
#define RFMOT_MSG2F " -%c %s %d "

/* adio second message format, for verbose option, total number
   of ampl., adc control status register, iod base address
   ex: "-v n_ampl= 8 adc_csr= 0xfff7c0 iod_bas= 0xfff000" */
#define RFAMP_MSG2F " -%c n_ampl= %d adc_csr= %s iod_bas= %s "
/* adio third message format, for ampl., dir. power address,
   refl. power address, and temperature address
   ex: "index= 1 dpower= 0xfff7A0 rpower= 0xfff7A2 temper= 0xfff7A4" */
#define RFAMP_MSG3F " index= %d dpower= %s rpower= %s temper= %s "

/* rf-controller second message format, for verbose option, tty name
   cryo number and qwr number
   ex: "-v /dev/tty11 1 1 " */
#define RFCON_MSG2F " -%c %s %d %d "

/* adio second message format, for verbose option,
   base address of aout board, base address of dout board,
   base address of ain board, base address of dinout board ex:
   "-v aout_ba= 0x00fff000 dout_ba= 0x00fff400
       ain_ba= 0x00fff800   dio_ba= 0x00fffC00" */
/* future develop.: # define HEADIO_MSG2F "\ 
-%c aout_ba= %s dout_ba= %s ain_ba= %s dio_ba= %s" */

#define HEADIO_MSG2F "\
-%c aout_ba= %s dout_ba= %s ain_ba= %s"

/* fast temperature measure device second message format,
   for verbose option and tty name
   ex: "-v /dev/tty17"  or "-n /dev/tty17" */
#define HEFTHER_MSG2F " -%c %s "

/* slow temperature measure device second message format,
   for verbose option and tty name
   ex: "-v /dev/tty17"  or "-n /dev/tty17" */
#define HESTHER_MSG2F " -%c %s "

/* img pressure measure device second message format,
   for verbose option and tty name
   ex: "-v /dev/tty17"  or "-n /dev/tty17" */
#define HEIMG_MSG2F " -%c %s "

