#pragma once

#include "rawchar.h"

#include <istream>
#include <ios>

namespace mfuse
{
	struct membuf : std::streambuf
	{
		membuf(const rawchar_t* begin, const rawchar_t* end) {
			this->setg((rawchar_t*)begin, (rawchar_t*)begin, (rawchar_t*)end);
		}

		pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode) override
		{
			switch (dir)
			{
			default:
			case std::ios_base::beg:
				this->setg(this->eback(), this->eback() + off, this->egptr());
				break;
			case std::ios_base::cur:
				this->setg(this->eback(), this->gptr() + off, this->egptr());
				break;
			case std::ios_base::end:
				this->setg(this->eback(), this->egptr() - off, this->egptr());
				break;
			}

			return this->gptr() - this->eback();
		}

		pos_type seekpos(pos_type off, std::ios_base::openmode) override
		{
			this->setg(this->eback(), this->eback() + off, this->egptr());
			return this->gptr() - this->eback();
		}
	};

	class memstream : public std::istream
	{
	public:
		memstream(const rawchar_t* buf, size_t length)
			: std::istream(&mem)
			, mem(buf, buf + length)
		{
			rdbuf(&mem);
		}

	private:
		membuf mem;
	};
}
