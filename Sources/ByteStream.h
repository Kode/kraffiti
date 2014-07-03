#pragma once

//#include "Path.h"
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
			for (byte b : bytes) {
				this->bytes.push_back(b);
				++index;
			}
		}

		size_t size() {
			return bytes.size();
		}

		//void save(kmd::Path path) {
		//	std::ofstream f(path.toString(), std::ios::binary);
		//	for (size_t i = 0; i < bytes.size(); ++i) f.put(bytes[i]);
		//}
	
		int index;
		std::vector<byte> bytes;
	};
}
