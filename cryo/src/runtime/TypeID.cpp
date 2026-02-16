#include "cryopch.h"
#include "TypeID.h"

namespace cryo::runtime {

	static std::unordered_map<std::string, TypeID> s_BuiltInTypes = {
		{ "void", VOID },

		{ "bool", BOOL },
		{ "char", CHAR },
		{ "u8", U8 },
		{ "i8", I8 },

		{ "u16", U16 },
		{ "i16", I16 },

		{ "u32", U32 },
		{ "i32", I32 },
		{ "f32", F32 },

		{ "u64", U64 },
		{ "i64", I64 },
		{ "f64", F64 }
	};

	size_t get_type_size(TypeID type) {
		switch (type) {
		case VOID: return 0;

		case BOOL:
		case CHAR:
		case U8:
		case I8: return sizeof(uint8_t);

		case U16:
		case I16: return sizeof(uint16_t);

		case U32:
		case I32:
		case F32: return sizeof(uint32_t);

		case U64:
		case I64:
		case F64: return sizeof(uint64_t);

		case REF: return sizeof(uint64_t) * 2; // TODO: 128 uuid

		default:
			throw std::runtime_error("Unknown TypeID");
		}
	}

	std::optional<TypeID> get_type_from_string(const std::string& type)
	{
		auto ite = s_BuiltInTypes.find(type);
		if (ite == s_BuiltInTypes.end()) {
			return {};
		}

		return ite->second;
	}

}