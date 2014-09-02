#pragma once

#include <fstream>
#include <vector>

namespace kake {
	typedef unsigned char byte;

	class ByteStream {
	public:
		ByteStream() : index(0) { }

		void set(int index, byte data) {
			bytes[index] = data;
		}

		void put(byte data) {
			bytes.push_back(data);
			++index;
		}

		void write(std::vector<byte> bytes) {
			for (size_t i = 0; i < bytes.size(); ++i) {
				this->bytes.push_back(bytes[i]);
				++index;
			}
		}

		size_t size() {
			return bytes.size();
		}

		void save(const char* path) {
			std::ofstream f(path, std::ios::binary);
			for (size_t i = 0; i < bytes.size(); ++i) f.put(bytes[i]);
		}

		void seek(int to) {
			index = to;
		}

		int index;
		std::vector<byte> bytes;
	};
}
