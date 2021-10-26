#pragma once

#include "rawchar.h"

#include <istream>
#include <ios>

namespace mfuse
{
	struct imembuf : std::streambuf
	{
		imembuf(const rawchar_t* begin, const rawchar_t* end) {
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
			this->setg(this->eback(), this->eback() + (size_t)off, this->egptr());
			return this->gptr() - this->eback();
		}
	};

	struct omembuf : std::streambuf
	{
		omembuf(rawchar_t* begin, rawchar_t* end)
			: buf(begin)
		{
			this->setp((rawchar_t*)begin, (rawchar_t*)end);
		}

		pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode) override
		{
			switch (dir)
			{
			default:
			case std::ios_base::beg:
				this->setp(buf + off, this->epptr());
				break;
			case std::ios_base::cur:
				this->setp(this->pptr() + off, this->epptr());
				break;
			case std::ios_base::end:
				this->setp(this->epptr() - off, this->epptr());
				break;
			}

			return this->pptr() - buf;
		}

		pos_type seekpos(pos_type off, std::ios_base::openmode) override
		{
			this->setp(buf + (size_t)off, this->epptr());
			return this->pptr() - buf;
		}

	private:
		rawchar_t* buf;
	};

	class imemstream : public std::istream
	{
	public:
		imemstream(const rawchar_t* buf, size_t length)
			: std::istream(&mem)
			, mem(buf, buf + length)
		{
			rdbuf(&mem);
		}

	private:
		imembuf mem;
	};

	class omemstream : public std::ostream
	{
	public:
		omemstream(rawchar_t* buf, size_t length)
			: std::ostream(&mem)
			, mem(buf, buf + length)
		{
			rdbuf(&mem);
		}

	private:
		omembuf mem;
	};
}
