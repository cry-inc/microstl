#include <microstl.h>
#include <iostream>

int main()
{
	struct MeshReceiver : microstl::IReceiver
	{
		void receiveFace(double v1[3], double v2[3], double v3[3]) override
		{
			std::cout << "Face: " << v1[0] << "|" << v1[1] << "|" << v1[2] << ", "
				<< v2[0] << "|" << v2[1] << "|" << v2[2] << ", "
				<< v3[0] << "|" << v3[1] << "|" << v3[2] << std::endl;
		}
	};

	MeshReceiver receiver;
	std::filesystem::path stlPath = "../../../testdata/simple_ascii.stl";
	microstl::Result result = microstl::parseStlFile(stlPath, receiver);
	std::cout << "Result: " << uint16_t(result) << std::endl;

	return 0;
}
