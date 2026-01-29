#include <morfuse/Script/MessageableException.h>

using namespace mfuse;

void Messageable::fill(const str& msg) const
{
    // fill the message for the first time
    Messageable* This = const_cast<Messageable*>(this);
    This->msg = msg;
}

bool Messageable::filled() const noexcept
{
    return !msg.isEmpty();
}

const char* Messageable::what() const noexcept
{
    return msg.c_str();
}
