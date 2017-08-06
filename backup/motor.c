#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

#include "motor.h"
#include "pin.h"

//global funtions declreation
void motor_setpid(float kp, float ki, float kd);


//private variations
volatile motor_typedef l_motor;
volatile motor_typedef r_motor;
volatile drive_typedef drive;

//private functions declearation
static void m_typereset(void);
static void m_rightcnt(void);
static void m_leftcnt(void);
static void *m_pid_thread(void *dummy);

int motor_init(void)
{
	if (wiringPiSetup() == -1)
	{
		printf("setup error\n");
		return 1;
	}
	
	pinMode(R_PWM,PWM_OUTPUT);
	pinMode(R_INF, OUTPUT);
	pinMode(R_INR, OUTPUT);
	pinMode(R_CNT, INPUT);
	pinMode(L_PWM, PWM_OUTPUT);
	pinMode(L_INF, OUTPUT);
	pinMode(L_INR, OUTPUT);
	pinMode(L_CNT, INPUT);
	
	digitalWrite(R_INF, LOW);
	digitalWrite(R_INR, LOW);
	digitalWrite(L_INF, LOW);
	digitalWrite(L_INR, LOW);
	
	wiringPiISR(R_CNT, INT_EDGE_BOTH, m_rightcnt);
	wiringPiISR(L_CNT, INT_EDGE_BOTH, m_leftcnt);

	if (softPwmCreate(R_PWM, 0, 100))
	{																					return 1;																	}
																					if (softPwmCreate(L_PWM, 0, 100))
	{
		return 1;
	}

	m_typereset();
	motor_setpid(0.4, 0.02, 0.1);

	piThreadCreate(m_pid_thread);

	return 0;	
}

static void m_typereset(void)
{
    drive.mode = M_BREAK;
	drive.spd  = 0;
	drive.dirc = 0;

    r_motor.spd = 0;

    r_motor.inte    = 0;
	r_motor.diff    = 0;
	r_motor.error   = 0;
	r_motor.p_error = 0;
	r_motor.pwmout  = 0;

    l_motor.spd = 0;

	l_motor.inte    = 0;
	l_motor.diff    = 0;
	l_motor.error   = 0;
	l_motor.p_error = 0;
	l_motor.pwmout  = 0;											
}

static void m_break(void)
{
	digitalWrite(R_INF,LOW);
	digitalWrite(R_INR,LOW);
	digitalWrite(L_INF,LOW);
	digitalWrite(L_INR,LOW);
}
																			
static void m_forward(void)
{
	digitalWrite(R_INF,HIGH);
	digitalWrite(R_INR,LOW);
	digitalWrite(L_INF,HIGH);
	digitalWrite(L_INR,LOW);
}

static void m_reverse(void)
{
	digitalWrite(R_INF,LOW);
	digitalWrite(R_INR,HIGH);
	digitalWrite(L_INF,LOW);
	digitalWrite(L_INR,HIGH);
}

static int m_setspd(volatile motor_typedef *motor)
{
	motor->error = motor->spd - motor->cnt;
	motor->cnt = 0;
	motor->inte += motor->error;
	motor->diff  = motor->error - motor->p_error;
	
	motor->pwmout += motor->kp * motor->error;
	motor->pwmout += motor->ki * motor->inte;
	motor->pwmout += motor->kd * motor->diff;
	if (motor->pwmout > 80)
	{
		motor->pwmout = 80;
	}
	else if (motor->pwmout < 0)
	{
		motor->pwmout = 0;
	}

	motor->p_error = motor->error;
	return motor->pwmout;
}

static void m_rightcnt(void)
{
	r_motor.cnt++;
}

static void m_leftcnt(void)
{
    l_motor.cnt++;
}

static void m_setrightspd(int spd)
{
	if (r_motor.ctrlmthd == OPEN_LOOP)
	{
		softPwmWrite(R_PWM, spd);
	}
	else if (r_motor.ctrlmthd == CLOSED_LOOP)
	{
		r_motor.spd = spd;
		softPwmWrite(R_PWM, m_setspd(&r_motor));
	}
}

static void m_setleftspd(int spd)
{
    if (r_motor.ctrlmthd == OPEN_LOOP)
    {
        softPwmWrite(L_PWM, spd);
    }
    else if (r_motor.ctrlmthd == CLOSED_LOOP)
    {
		l_motor.spd = spd;
        softPwmWrite(L_PWM, m_setspd(&l_motor));
    }
}

static void m_ctrl(volatile drive_typedef *drive)
{
    int l_spd = 0, r_spd = 0, dirc = 0;

	if (drive->mode == M_BREAK)
    {
	    m_setrightspd(0);
        m_setleftspd(0);
		m_break();
		m_typereset();
	}
	else if (drive->mode == M_FORWARD)
	{
		m_setrightspd(0);
		m_setleftspd(0);														        m_forward();
	}
	else if (drive->mode == M_REVERSE)
	{
		m_setrightspd(0);
		m_setleftspd(0);
		m_reverse();
	}

	r_spd = drive->spd;
	l_spd = drive->spd;
	dirc  = drive->dirc;
	//postive -> right, nagetive -> left
    if (dirc >= 0)
	{
		r_spd -= dirc;
		if (r_spd < 0)
		{
			r_spd = 0;	
		}
	}
    else
	{
		l_spd += dirc;
		if (l_spd < 0)
		{
			l_spd = 0;
		}	
	}

	m_setrightspd(r_spd);
	m_setleftspd(l_spd);
}


static void *m_pid_thread(void *dummy)
{
	volatile unsigned int cr_ptime = 0, cr_ltime = 0;
	uint8_t timer_100ms_flag = 0, timer_1s_flag = 0;

	cr_ptime = millis();
	while (1)
	{
		delay(5);	
		cr_ltime = millis();
		if (cr_ltime - cr_ptime >= 100)
		{
			cr_ptime = millis();
			timer_100ms_flag = 1;			
		}
		
		if (timer_100ms_flag == 1)
		{
			timer_100ms_flag = 0;
			timer_1s_flag ++;
			m_ctrl(&drive);
			//m_setrightspd(20);
		}

		if (timer_1s_flag == 10)
		{
			timer_1s_flag = 0;
		}		
	}
	return NULL;
}

void motor_setpid(float kp, float ki, float kd)
{
	r_motor.ctrlmthd = CLOSED_LOOP;
	r_motor.kp = kp;
	r_motor.ki = ki;
	r_motor.kd = kd;

	l_motor.ctrlmthd = CLOSED_LOOP;
	l_motor.kp = kp;
	l_motor.ki = ki;
	l_motor.kd = kd;
}

void motor_ctrl(uint8_t sts, int spd, int dirc)
{
	drive.mode = sts;
	drive.spd  = spd;
	drive.dirc = dirc;
}


int main(void)
{
	int s = 0, d = 0, m = 0;
	if (motor_init() == 1)
	{
		return 0;
	}	

	while (1)
	{
		//printf("r=%d, l=%d\n", r_motor.pwmout, l_motor.pwmout);
		//r_motor.pwmout = 0;
		//l_motor.pwmout = 0;
		delay(100);
		printf("s, d, m\n");
		scanf("%d,%d,%d", &s, &d, &m);
		
		printf("s=%d, d=%d, m=%d\n", s, d, m);
		drive.spd = s;
		drive.dirc = d;
		drive.mode = m;
		
		if (m == 0)
		{
			m_break();
		}
		else if (m == 1)
		{
			m_forward();
		}
		else if (m == 2)
		{
			m_reverse();
		}
	}
	
	return 1;	
}
