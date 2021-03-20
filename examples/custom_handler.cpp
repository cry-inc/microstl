#include <microstl.h>

#include <iostream>

struct CustomHandler : microstl::Reader::Handler
{
	void onBegin(bool asciiMode) override
	{
		std::cout << "Begin parsing with " << (asciiMode ? "ASCII" : "binary") << " mode" << std::endl;
	}

	void onBinaryHeader(const uint8_t header[80]) override
	{
		std::cout << "Header: ";
		for (size_t i = 0; i < 80; i++)
			std::cout << int(header[i]) << ' ';
		std::cout << std::endl;
	}

	void onName(const std::string& name) override
	{
		std::cout << "Name: " << name << std::endl;
	}

	void onFacetCount(uint32_t triangles) override
	{
		std::cout << "Triangles: " << triangles << std::endl;
	}

	void onError(size_t lineNumber) override
	{
		std::cout << "Error detected on line: " << lineNumber << std::endl;
	}

	void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) override
	{
		std::cout << "Facet: " << n[0] << "|" << n[1] << "|" << n[2] << ", "
			<< v1[0] << "|" << v1[1] << "|" << v1[2] << ", "
			<< v2[0] << "|" << v2[1] << "|" << v2[2] << ", "
			<< v3[0] << "|" << v3[1] << "|" << v3[2] << std::endl;
	}

	void onFacetAttributes(const uint8_t attribute[2]) override
	{
		std::cout << "Attributes: " << int(attribute[0]) << ' ' << int(attribute[1]) << std::endl;
	}

	void onEnd(microstl::Result result) override
	{
		std::cout << "Finished parsing with return value " << microstl::getResultString(result) << std::endl;
	}
};

int main()
{
	/// Create an instance of the custom handler that will receive the STL data
	CustomHandler handler;

	// Parse STL file with the custom handler as receiver
	std::filesystem::path filePath = "../../testdata/simple_ascii.stl";
	microstl::Result result = microstl::Reader::readStlFile(filePath, handler);
	
	// Error handling
	if (result != microstl::Result::Success)
	{
		std::cout << "Error: " << microstl::getResultString(result) << std::endl;
		return 1;
	}

	return 0;
}
