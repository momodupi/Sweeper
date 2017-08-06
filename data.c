#include <stdio.h>
#include <stdint.h>

#include "data.h"


int socket_unpack(socket_typedef *dp, sweeper_typedef *sp)
{
	int pn = 0;

	if (dp->type != 0)
	{
		return -1;	
	}
	
	if ((dp->data.part.head[0] != '@')
		|| (dp->data.part.head[1] != '@')
		|| (dp->data.part.tail[0] != '!')
		|| (dp->data.part.tail[1] != '!'))
	{
		return -2;	
	}

	if ((dp->data.part.thead == 'T') 
		&& (dp->data.part.ttail == 't'))
	{
		sp->sweeper_mode = (uint8_t)(dp->data.part.tcmd - '0');	
	}
	
    if ((dp->data.part.mhead == 'M')
        && (dp->data.part.mtail == 'm'))
    {
        sp->motor_mode = (uint8_t)(dp->data.part.mcmd - '0');
    }

    if ((dp->data.part.shead == 'S')
	    && (dp->data.part.stail == 's'))
	{
		sp->motor_spd  = ((int)(dp->data.part.scmd[0] - '0')) * 100;
		sp->motor_spd += ((int)(dp->data.part.scmd[1] - '0')) * 10;
		sp->motor_spd +=  (int)(dp->data.part.scmd[2] - '0');
	}
	 
	if ((dp->data.part.dhead == 'D')
		&& (dp->data.part.dtail == 'd'))
	{
		if (dp->data.part.dcmd[0] == '-')
		{
			pn = -1;
		}
		else if (dp->data.part.dcmd[0] == '+')
		{
			pn = 1;
		}
		else
		{
			pn = 0;
		}
		sp->motor_dirc  = ((int)(dp->data.part.dcmd[1] - '0')) * 10;
		sp->motor_dirc +=  (int)(dp->data.part.dcmd[2] - '0');
		sp->motor_dirc *= pn;
	}

	if ((dp->data.part.chead == 'C')
		&& (dp->data.part.ctail == 'c'))
	{
		sp->camera_mode = (uint8_t)(dp->data.part.ccmd - '0');
	}
	return 0;
}

int socket_pack(socket_typedef *dp, sweeper_typedef *sp)
{
	dp->data.part.head[0] = '@';
	dp->data.part.head[1] = '@';

	dp->data.part.thead = 'T';
	dp->data.part.tcmd = sp->sweeper_mode + '0';
	dp->data.part.ttail = 't';

	dp->data.part.mhead = 'M';
    dp->data.part.mcmd = sp->motor_mode + '0';
    dp->data.part.mtail = 'm';

	dp->data.part.shead = 'S';
    dp->data.part.scmd[0] =  (sp->motor_spd / 100) + '0';
	dp->data.part.scmd[1] = ((sp->motor_spd % 100) / 10) + '0';
	dp->data.part.scmd[2] =  (sp->motor_spd % 10) + '0';
    dp->data.part.stail = 's';

    dp->data.part.dhead = 'D';
	if (sp->motor_dirc < 0)
	{
		dp->data.part.dcmd[0] = '-';
		dp->data.part.dcmd[1] = (sp->motor_dirc * (-1) / 10) + '0';
	    dp->data.part.dcmd[2] = (sp->motor_dirc * (-1) % 10) + '0';
	}
	else if (sp->motor_dirc == 0)
	{
		dp->data.part.dcmd[0] = '+';
		dp->data.part.dcmd[1] = '0';
		dp->data.part.dcmd[2] = '0';
	}
	else
	{
		dp->data.part.dcmd[0] = '+';
		dp->data.part.dcmd[1] = (sp->motor_dirc / 10) + '0';
		dp->data.part.dcmd[2] = (sp->motor_dirc % 10) + '0';
	}
    dp->data.part.dtail = 'd';

	dp->data.part.chead = 'C';
    dp->data.part.ccmd = sp->camera_mode + '0';
	dp->data.part.ctail = 'c';

	dp->data.part.tail[0] = '!';
	dp->data.part.tail[1] = '!';
	dp->data.part.end = '\0';

	return 0;
}
