#include <microstl.h>

#include <iostream>

int main()
{
	// Define path to input file
	std::filesystem::path filePath = "../../testdata/simple_ascii.stl";
	
	// Use included handler that creates a simple mesh data structure
	microstl::MeshReaderHandler meshHandler;

	// Start parsing the file and let the data go into the mesh handler
	microstl::Result result = microstl::Reader::readStlFile(filePath, meshHandler);
	
	// Check if the parsing was successful or if there was an error
	if (result != microstl::Result::Success)
	{
		std::cout << "Error: " << microstl::getResultString(result) << std::endl;
		return 1;
	}
	
	// Now the extracted mesh data can be accessed
	const microstl::Mesh& mesh = meshHandler.mesh;

	// Loop over all triangles a.k.a facets
	for (const microstl::Facet& facet : mesh.facets)
	{
		std::cout << "Normal: " << facet.n.x << ' ' << facet.n.y << ' ' << facet.n.z << std::endl;
		std::cout << "Vertex 1: " << facet.v1.x << ' ' << facet.v1.y << ' ' << facet.v1.z << std::endl;
		std::cout << "Vertex 2: " << facet.v2.x << ' ' << facet.v2.y << ' ' << facet.v2.z << std::endl;
		std::cout << "Vertex 3: " << facet.v3.x << ' ' << facet.v3.y << ' ' << facet.v3.z << std::endl;
	}

	// The handler also collected some other information
	std::cout << "Mesh Name: " << meshHandler.name << std::endl;
	std::cout << "ASCII: " << (meshHandler.ascii ? "true" : "false") << std::endl;

	return 0;
}
