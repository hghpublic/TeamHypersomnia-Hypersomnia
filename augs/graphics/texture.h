#pragma once
#include "augs/image/image.h"

namespace augs {
	namespace graphics {
		struct texture {
			unsigned int id = 0xdeadbeef;
			bool built = false;

			texture() = default;
			~texture();

			texture(const texture&) = delete;
			texture(texture&&) = delete;
			texture& operator=(const texture&) = delete;
			texture& operator=(texture&&) = delete;

			void create(const augs::image& source);
			void destroy();
		};
	}
}