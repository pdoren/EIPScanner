//
// Created by Aleksey Timin on 12/4/19.
//

#ifndef EIPSCANNER_CIP_STRINGS_H
#define EIPSCANNER_CIP_STRINGS_H

#include <cstring>
#include <string>
#include <vector>
#include "Types.h"

namespace eipScanner {
namespace cip {
	template <typename T>
	class CipBaseString {
	public:
		CipBaseString() = default;
		explicit CipBaseString(const std::string& string) {
			_length = string.size();
			uint8_t *buffer = new uint8_t[_length];
			std::memcpy(buffer, string.data(), _length);

			_data = std::vector<uint8_t>(buffer, buffer + _length);
			delete[] buffer;
		}

		CipBaseString(const std::vector<uint8_t>& data) {
			_length = data.size();
			_data = data;
		}

		~CipBaseString() = default;

		std::string toStdString() const {
			char *buffer = new char[_length];
			std::memcpy(buffer, _data.data(), _length);
			std::string buf(buffer, buffer + _length);
			delete[] buffer;
			return buf;
		}

		T getLength() const {
			return _length;
		}

		const std::vector<uint8_t> &getData() const {
			return _data;
		}

	private:
		T _length;
		std::vector<uint8_t> _data;
	};

	using CipShortString = CipBaseString<CipUsint>;
	using CipString = CipBaseString<CipUint>;
}
}

#endif  // EIPSCANNER_STRINGS_H
