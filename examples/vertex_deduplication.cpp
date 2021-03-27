#include <microstl.h>

#include <iostream>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		// The recommended test file is box_meshlab_ascii.stl
		std::cerr << "Missing argument for input file!" << std::endl;
		return 1;
	}

	std::filesystem::path filePath(argv[1]);
	microstl::MeshReaderHandler meshHandler;
	microstl::Result result = microstl::Reader::readStlFile(filePath, meshHandler);
	if (result != microstl::Result::Success)
	{
		std::cerr << "Error: " << microstl::getResultString(result) << std::endl;
		return 1;
	}

	// The mesh returned by the MeshParserHandler still contains duplicated vertices
	// since each facet contains a copy of all three vertices.
	const microstl::Mesh& duplicatedVerticesMesh = meshHandler.mesh;

	// Use the library to create a deuplicated vertex list and facets with
	// vertex indices rather than full coordinate values (note the type prefix "FV").
	microstl::FVMesh deduplicatedVerticesMesh = microstl::deduplicateVertices(duplicatedVerticesMesh);

	// In case of our example cube with 2 triangles for each side we have 6 * 2 = 12 triangles.
	// Since each triangle had three vertices, we started with 3 * 12 = 36 vertices.
	// After the deduplication we should have only 8 (one for each corner of the cube).
	std::cout << "Old Vertex Count: " << duplicatedVerticesMesh.facets.size() * 3 << std::endl;
	std::cout << "New Vertex Count: " << deduplicatedVerticesMesh.vertices.size() << std::endl;

	return 0;
}
