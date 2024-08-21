
#include "tickcount.h"

namespace su
{

size_t TickCount::getCount(bool reset)
{
    size_t oldtick = m_tick;
    size_t curtick = getTickCount();

	if(reset)
	{
        m_tick = curtick;
	}

	return curtick - oldtick;
}

size_t TickCount::getTickCount()
{
    return duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now().time_since_epoch()).count();
}

size_t TickCount::unixTime()
{
    return duration_cast<std::chrono::seconds>
        (std::chrono::steady_clock::now().time_since_epoch()).count();
}

}
