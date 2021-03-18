#include <microstl.h>
#include <iostream>

int main()
{
	struct MeshReceiver : microstl::IReceiver
	{
		void receiveName(const std::string& name) override
		{
			std::cout << "Name: " << name << std::endl;
		}

		void receiveFace(const double v1[3], const double v2[3], const double v3[3], const double n[3]) override
		{
			std::cout << "Face: " << v1[0] << "|" << v1[1] << "|" << v1[2] << ", "
				<< v2[0] << "|" << v2[1] << "|" << v2[2] << ", "
				<< v3[0] << "|" << v3[1] << "|" << v3[2] << ", "
				<< n[0] << "|" << n[1] << "|" << n[2] << std::endl;
		}
	};

	MeshReceiver receiver;
	std::filesystem::path stlPath = "../../../testdata/simple_ascii.stl";
	microstl::Result result = microstl::parseStlFile(stlPath, receiver);
	std::cout << "Result: " << uint16_t(result) << std::endl;

	return 0;
}
