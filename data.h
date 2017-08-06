#ifndef __DATA_H__
#define __DATA_H__

#define DATASIZE  24

typedef struct
{
	uint8_t sweeper_mode;
	uint8_t motor_mode;
	int motor_spd;
	int motor_dirc;
	uint8_t camera_mode;
}sweeper_typedef;


typedef struct
{
	int type;

	union
	{
		char buf[DATASIZE];
		struct
		{
			char head[2];

			char thead;
			char tcmd;
			char ttail;

			char mhead;
			char mcmd;
			char mtail;
				
			char shead;
			char scmd[3];
			char stail;

			char dhead;
			char dcmd[3];
			char dtail;

			char chead;
			char ccmd;
			char ctail;

			char tail[2];
			char end;
		}part;
	}data;

}socket_typedef;

extern int socket_unpack(socket_typedef *dp, sweeper_typedef *sp);
extern int socket_pack(socket_typedef *dp, sweeper_typedef *sp);


#endif

