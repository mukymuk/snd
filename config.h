#define CONFIG_VERSION 1000

#define AXIS_COUNT	2
#define SPEED_ACC_COUNT	16


typedef struct
{
	float_t	speed;
	float_t acceleration;
}
config_sa_t;

typedef struct
{
	float_t			speed;
	float_t			step[AXIS_COUNT];
	config_sa_t		speed_acc[AXIS_COUNT][SPEED_ACC_COUNT];
}
config_t;

extern config_t g_config;
