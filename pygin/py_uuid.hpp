#pragma once

#include "py_object.hpp"

namespace py
{
	class uuid: public object
	{
	public:
		static auto type_name() { return "UUID"; }

		explicit uuid(const UUID& Uuid);
		uuid(cast_guard, const object& Object);

		UUID to_uuid() const;
	};
}
