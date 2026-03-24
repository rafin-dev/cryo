#pragma once

namespace cryo::runtime {

	using CryoValue = std::variant<
		bool,
		int64_t,
		double,
	
		std::string
	>;

}