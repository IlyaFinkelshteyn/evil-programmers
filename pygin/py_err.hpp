#pragma once

#include "py_type.hpp"

namespace py
{
	namespace err
	{
		object occurred();
		void clear();
		void print();

		class exception: public type
		{
		public:
			explicit exception(const char* Name);
		};
	}
}
