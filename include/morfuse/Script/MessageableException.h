#pragma once

#include "../Common/str.h"

namespace mfuse
{
	class Messageable
	{
	public:
		void fill(const str& msg) const;
		bool filled() const noexcept;

	protected:
		const char* what() const noexcept;

	private:
		str msg;
	};
}
