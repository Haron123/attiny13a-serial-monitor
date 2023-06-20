#include "../Header/time.h"

void init_system_tick()
{
	// Normal operation
	TCCR0A = 0;
	// Set Prescaler to 8
	TCCR0B = (1 << CS01);
}

tTime time_now()
{
	return TCNT0;
}

tTime time_passed(tTime since)
{
	tTime now = time_now();
	if(now >= since)
	{
		return (now - since);
	}
	else
	{
		// rollover has occured
		return (now + (1 + MAX_TIME - since));
	}
}