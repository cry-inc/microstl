#include <microstl.h>

#include <iostream>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		// The recommended test file is simple_ascii.stl
		std::cerr << "Missing argument for input file!" << std::endl;
		return 1;
	}

	std::filesystem::path filePath(argv[1]);
	microstl::MeshReaderHandler meshHandler;
	microstl::Result result = microstl::Reader::readStlFile(filePath, meshHandler);
	if (result != microstl::Result::Success)
	{
		std::cerr << "Reading Error: " << microstl::getResultString(result) << std::endl;
		return 1;
	}

	if (meshHandler.ascii == false)
		std::cout << "Warning: Input file is already a binary STL file!" << std::endl;

	auto folder = filePath.parent_path();
	auto newFileName = filePath.stem().string() + "_binary.stl";
	auto newPath = folder / newFileName;

	microstl::MeshProvider provider(meshHandler.mesh);
	result = microstl::Writer::writeStlFile(newPath, provider);
	if (result != microstl::Result::Success)
	{
		std::cerr << "Writing Error: " << microstl::getResultString(result) << std::endl;
		return 1;
	}

	std::cout << "Finished converting " << filePath.filename() << " into binary STL file " << newPath.filename() << std::endl;

	return 0;
}
