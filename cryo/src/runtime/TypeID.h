#pragma once

namespace cryo::runtime {

	enum TypeID {
		VOID,

		BOOL,
		CHAR,
		U8,
		I8,

		U16,
		I16,

		U32,
		I32,
		F32,

		U64,
		I64,
		F64,

		REF
	};

	// Just to make sure the stack will use the appropriate size
	static_assert(sizeof(uint32_t) == sizeof(float));
	static_assert(sizeof(uint64_t) == sizeof(double));
	size_t get_type_size(TypeID type);

	std::optional<TypeID> get_type_from_string(const std::string& type);

	using ExpressionResult = std::variant<
		bool,
		char,
		uint8_t,
		int8_t,

		uint16_t,
		int16_t,

		uint32_t,
		int32_t,
		float,

		uint64_t,
		int64_t,
		double,
	
		std::string
	>;

}