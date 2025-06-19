#include "bitmap.h"

namespace FF {

	class Tools {
	public:

		static glm::vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize);
		static Bitmap convertEquirectangularToCubemapFaces(const Bitmap& equirect);

	};

}






