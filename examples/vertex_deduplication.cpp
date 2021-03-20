#include <microstl.h>

#include <iostream>

int main()
{
	std::filesystem::path filePath = "../../testdata/box_meshlab_ascii.stl";

	microstl::MeshParserHandler meshHandler;
	microstl::Result result = microstl::Parser::parseStlFile(filePath, meshHandler);
	if (result != microstl::Result::Success)
	{
		std::cout << "Error: " << microstl::getResultString(result) << std::endl;
		return 1;
	}

	// The mesh returned by the MeshParserHandler still contains duplicated vertices
	// since each facet contains a copy of all three vertices.
	const microstl::Mesh& duplicatedVerticesMesh = meshHandler.mesh;

	// Use the library to create a deuplicated vertex list and facets with
	// vertex indices rather than full coordinate values (note the type prefix "FV").
	microstl::FVMesh deduplicatedVerticesMesh = microstl::deduplicateVertices(duplicatedVerticesMesh);

	// Since our example file is a cube with 2 triangles for each side we have 6 * 2 = 12 triangles.
	// Since each triangle had three vertices, we started with 3 * 12 = 36 vertices.
	// After the deduplication we should have only 8 (one for each corner of the cube).
	std::cout << "Old Vertex Count: " << duplicatedVerticesMesh.facets.size() * 3 << std::endl;
	std::cout << "New Vertex Count: " << deduplicatedVerticesMesh.vertices.size() << std::endl;

	return 0;
}
