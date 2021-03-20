// microstl.h - STL file format parser - under MIT License
// See https://github.com/cry-inc/microstl for details

#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <streambuf>
#include <vector>
#include <exception>

namespace microstl
{
	// Possible return values
	enum class Result : uint16_t {
		Undefined = 0, // Will be never returned by the parser and can be used the to indicate preding or empty results
		Success = 1, // Everything went smooth, the STL file was parsed without issues
		FileError = 2, // Unable to read or write the specified file path
		MissingDataError = 3, // STL data ended unexpectely and is incomplete or otherwise broken
		UnexpectedError = 4, // Found an unexpected keyword or token in an ASCII STL file
		ParserError = 5, // Unable to parse vertex coordinates or normal vector in an ASCII STL file
		LineLimitError = 6, // ASCII line size exceeded internal safety limit of ASCII_LINE_LIMIT
		FacetCountError = 7, // Binray file exceeds internal safety limit of BINARY_FACET_LIMIT
		EndianError = 8, // The code currently only supports little endian architectures
		__LAST__RESULT__VALUE = 9 // Only used for automated checks
	};

	class Parser
	{
	public:
		// Interface that must be implemented to receive the data from the parsed STL file
		class Handler
		{
		public:
			virtual ~Handler() {}

			// Called when the parsing is started before any other methods
			virtual void onBegin(bool asciiMode) {}

			// Called with the header bytes of a binary STL file after onBinary()
			virtual void onBinaryHeader(const uint8_t header[80]) {}

			// Always called when parsing a binary STL. Before onFacet() is called for the first time
			virtual void onFacetCount(uint32_t triangles) {}

			// May be called when parsing an ASCII STL file with a valid name. Will be always called before onFacet()
			virtual void onName(const std::string& name) {}

			// Might be called in ASCII mode when an error is detected to signal the line number of the problem
			// Do not rely on this method to be called when an error occurs, its fully optional!
			virtual void onError(size_t lineNumber) {}

			// Will be called for each triangle (a.k.a facet/face) in the STL file
			virtual void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) = 0;

			// Can be called for non-zero attribute values of facets in binary STL files after onFacet()
			virtual void onFacetAttributes(const uint8_t attributes[2]) {}

			// Called when the parsing process finishes after all other methods
			virtual void onEnd(Result result) {}
		};

		// Parse STL file directly from disk using an UTF8 or ASCII path
		static Result parseStlFile(const char* utf8FilePath, Handler& handler)
		{
			std::filesystem::path path = std::filesystem::u8path(utf8FilePath);
			return parseStlFile(path, handler);
		}

		// Parse STL file directly from disk using an wide string path
		static Result parseStlFile(const wchar_t* filePath, Handler& handler)
		{
			std::filesystem::path path(filePath);
			return parseStlFile(path, handler);
		}

		// Parse STL file directly from disk using a std::filesystem path
		static Result parseStlFile(const std::filesystem::path& filePath, Handler& handler)
		{
			std::ifstream ifs(filePath, std::ios::binary);
			if (!ifs)
			{
				auto result = Result::FileError;
				handler.onBegin(false);
				handler.onEnd(result);
				return result;
			}

			return parseStlStream(ifs, handler);
		};

		// Parse STL file from a memory buffer
		static Result parseStlBuffer(const char* buffer, size_t bufferSize, Handler& handler)
		{
			imstream stream(buffer, bufferSize);
			return parseStlStream(stream, handler);
		}

		// Parse STL file from a std::istream source
		static Result parseStlStream(std::istream& is, Handler& handler)
		{
			bool asciiMode = isAsciiFormat(is);
			handler.onBegin(asciiMode);
			Result result = asciiMode ? parseAsciiStream(is, handler) : parseBinaryStream(is, handler);
			handler.onEnd(result);
			return result;
		}

		// Some internal safety limits
		static inline const size_t ASCII_LINE_LIMIT = 512u;
		static inline const uint32_t BINARY_FACET_LIMIT = 500000000u;
		static inline const float NORMAL_LENGTH_DEVIATION_LIMIT = 0.001f;

	private:
		static bool isAsciiFormat(std::istream& is)
		{
			char header[5] = { 0, };
			is.read(header, sizeof(header));
			is.seekg(0, std::ios::beg);
			return memcmp("solid", header, sizeof(header)) == 0;
		}

		static bool readNextLine(std::istream& is, std::string& output)
		{
			output.resize(0);
			if (!is)
				return false;

			while (!is.eof())
			{
				char byte;
				is.read(&byte, 1);
				if (byte == '\n')
					return true;
				else if (output.size() > ASCII_LINE_LIMIT)
					return false;
				else
					output.push_back(byte);
			}

			return true;
		}

		static std::string stringTrim(const std::string& line)
		{
			std::string result;
			size_t index = 0;
			while (index < line.size() && (line[index] == '\t' || line[index] == ' ' || line[index] == '\r' || line[index] == '\n'))
			{
				index++;
			}

			if (index == line.size())
				return result;

			while (index < line.size())
			{
				result.push_back(line[index]);
				index++;
			}

			index = result.size() - 1;
			while (index >= 0 && (result[index] == '\t' || result[index] == ' ' || result[index] == '\r' || result[index] == '\n'))
			{
				index--;
			}

			result.resize(index + 1);
			return result;
		}

		static bool stringStartsWith(const std::string& str, const char* prefix)
		{
			return memcmp(prefix, str.data(), strlen(prefix)) == 0;
		}

		static bool stringParseThreeValues(const std::string& str, float& v1, float& v2, float& v3)
		{
			std::stringstream ss(str);
			ss >> v1;
			if (!ss)
				return false;

			ss >> v2;
			if (!ss)
				return false;

			ss >> v3;
			if (!ss)
				return false;

			return true;
		}

		static void calculateNormals(const float v1[3], const float v2[3], const float v3[3], float n[3])
		{
			float u[3] = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
			float v[3] = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };
			n[0] = u[1] * v[2] - u[2] * v[1];
			n[1] = u[2] * v[0] - u[0] * v[2];
			n[2] = u[0] * v[1] - u[1] * v[0];
		}

		static void checkAndFixNormals(const float v1[3], const float v2[3], const float v3[3], float n[3])
		{
			if (n[0] == 0 && n[1] == 0 && n[2] == 0)
				return calculateNormals(v1, v2, v3, n);

			float length = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
			if (fabs(length - 1.0f) > NORMAL_LENGTH_DEVIATION_LIMIT)
				return calculateNormals(v1, v2, v3, n);
		}

		static bool isLittleEndian()
		{
			int16_t number = 1;
			const char* ptr = reinterpret_cast<const char*>(&number);
			return ptr[0] == 1;
		}

		static Result parseAsciiStream(std::istream& is, Handler& handler)
		{
			// State machine variables
			bool activeSolid = false;
			bool activeFacet = false;
			bool activeLoop = false;
			size_t lineNumber = 0, solidCount = 0, facetCount = 0, loopCount = 0, vertexCount = 0;
			float n[3] = { 0, };
			float v[9] = { 0, };

			// Line parse with loop to work the state machine
			while (true)
			{
				lineNumber++;
				std::string line;
				if (!readNextLine(is, line))
				{
					if (is)
					{
						// input stream still good -> hit the line limit
						handler.onError(lineNumber);
						return Result::LineLimitError;
					}
					else
					{
						// input stream ended, no more lines!
						break;
					}
				}
				line = stringTrim(line);
				if (stringStartsWith(line, "solid"))
				{
					if (activeSolid || solidCount != 0)
					{
						handler.onError(lineNumber);
						return Result::UnexpectedError;
					}
					activeSolid = true;
					if (line.length() > 5)
					{
						std::string name = stringTrim(line.substr(5));
						handler.onName(name);
					}
				}
				if (stringStartsWith(line, "endsolid"))
				{
					if (!activeSolid || activeFacet || activeLoop)
					{
						handler.onError(lineNumber);
						return Result::UnexpectedError;
					}
					activeSolid = false;
					solidCount++;
				}
				if (stringStartsWith(line, "facet normal"))
				{
					if (!activeSolid || activeLoop || activeFacet)
					{
						handler.onError(lineNumber);
						return Result::UnexpectedError;
					}
					activeFacet = true;
					line = stringTrim(line.substr(12));
					if (!stringParseThreeValues(line, n[0], n[1], n[2]))
					{
						handler.onError(lineNumber);
						return Result::ParserError;
					}
				}
				if (stringStartsWith(line, "endfacet"))
				{
					if (!activeSolid || activeLoop || !activeFacet || loopCount != 1)
					{
						handler.onError(lineNumber);
						return Result::UnexpectedError;
					}
					activeFacet = false;
					facetCount++;
					loopCount = 0;
					checkAndFixNormals(v + 0, v + 3, v + 6, n);
					handler.onFacet(v + 0, v + 3, v + 6, n);
				}
				if (stringStartsWith(line, "outer loop"))
				{
					if (!activeSolid || !activeFacet || activeLoop)
					{
						handler.onError(lineNumber);
						return Result::UnexpectedError;
					}
					activeLoop = true;
				}
				if (stringStartsWith(line, "endloop"))
				{
					if (!activeSolid || !activeFacet || !activeLoop || vertexCount != 3)
					{
						handler.onError(lineNumber);
						return Result::UnexpectedError;
					}
					activeLoop = false;
					loopCount++;
					vertexCount = 0;
				}
				if (stringStartsWith(line, "vertex"))
				{
					if (!activeSolid || !activeFacet || !activeLoop || vertexCount >= 3)
					{
						handler.onError(lineNumber);
						return Result::UnexpectedError;
					}
					line = stringTrim(line.substr(6));
					if (!stringParseThreeValues(line, v[vertexCount * 3 + 0], v[vertexCount * 3 + 1], v[vertexCount * 3 + 2]))
					{
						handler.onError(lineNumber);
						return Result::ParserError;
					}
					vertexCount++;
				}
			}

			if (activeSolid || activeFacet || activeLoop || solidCount == 0)
				return Result::MissingDataError;

			return Result::Success;
		}

		static Result parseBinaryStream(std::istream& is, Handler& handler)
		{
			if (!isLittleEndian())
				return Result::EndianError;

			char buffer[80];
			is.read(buffer, sizeof(buffer));
			if (!is)
				return Result::MissingDataError;
			handler.onBinaryHeader(reinterpret_cast<const uint8_t*>(buffer));
			is.read(buffer, 4);
			if (!is)
				return Result::MissingDataError;
			uint32_t facetCount = reinterpret_cast<uint32_t*>(buffer)[0];
			if (facetCount == 0)
				return Result::MissingDataError;
			if (facetCount > BINARY_FACET_LIMIT)
				return Result::FacetCountError;
			handler.onFacetCount(facetCount);
			for (size_t t = 0; t < facetCount; t++)
			{
				is.read(buffer, 50);
				if (!is)
					return Result::MissingDataError;
				float values[12];
				memcpy(values, buffer, 4 * 12);
				checkAndFixNormals(values + 3, values + 6, values + 9, values);
				handler.onFacet(values + 3, values + 6, values + 9, values);
				if (buffer[48] != 0 || buffer[49] != 0)
					handler.onFacetAttributes(reinterpret_cast<const uint8_t*>(buffer + 48));
			}

			return Result::Success;
		}

		// Private helpers to convert a memory buffer into a seekable istream
		// See source here: https://stackoverflow.com/a/46069245
		struct membuf : std::streambuf
		{
			membuf(char const* base, size_t size)
			{
				char* p = const_cast<char*>(base);
				this->setg(p, p, p + size);
			}
		};
		struct imstream : virtual membuf, std::istream
		{
			imstream(char const* base, size_t size) : membuf(base, size), std::istream(static_cast<std::streambuf*>(this)) {}
			std::iostream::pos_type seekpos(std::iostream::pos_type sp, std::ios_base::openmode which) override
			{
				return seekoff(sp - std::iostream::pos_type(std::iostream::off_type(0)), std::ios_base::beg, which);
			}
			std::iostream::pos_type seekoff(std::iostream::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override
			{
				if (dir == std::ios_base::cur) gbump(static_cast<int>(off));
				else if (dir == std::ios_base::end) setg(eback(), egptr() + off, egptr());
				else if (dir == std::ios_base::beg) setg(eback(), eback() + off, egptr());
				return gptr() - eback();
			}
		};
	};

	class Writer
	{
	public:
		// Provider that must be implemented configure options and provide data when writing STL files
		class Provider
		{
		public:
			virtual ~Provider() {}

			// Return true to write an ASCII file, return false to write a binary file
			virtual bool asciiMode() { return false; }

			// Can be used to supply an name for ASCII STL files
			virtual std::string getName() { return libraryName; }

			// Can be used to provide a custom 80 byte header for binary STL files
			// The array header is an output parameter.
			virtual void getHeader(uint8_t header[80]) { memset(header, 0, 80); memcpy(header, libraryName, strlen(libraryName)); }

			// Return true to write nulled out normals, return false write existing normal data
			virtual bool nullifyNormals() { return false; }

			// Return true if you want to write custom attribute values in binary STL files using getFacetAttributes()
			virtual bool writeAttributes() { return false; }

			// Must return the number of facets/triangles that will go into the STL file
			// Will be called once before the first getFacet() call
			virtual size_t getFacetCount() = 0;

			// Will be called once for each facet/triangle with its corresponding zero based index
			// The arrays v1, v2, v3 and n are output parameters
			virtual void getFacet(size_t index, float v1[3], float v2[3], float v3[3], float n[3]) = 0;

			/// Will be called once for each facet/triangle after getFacet() if writeAttributes() is true
			// The array attributes is an output parameter
			virtual void getFacetAttributes(size_t index, uint8_t attributes[2]) { memset(attributes, 0, 2); }
		};

		// Write STL file directly to disk using an UTF8 or ASCII path
		static Result writeStlFile(const char* utf8FilePath, Provider& provider)
		{
			std::filesystem::path path = std::filesystem::u8path(utf8FilePath);
			return writeStlFile(path, provider);
		}

		// Write STL file directly to disk using an wide string path
		static Result writeStlFile(const wchar_t* filePath, Provider& provider)
		{
			std::filesystem::path path(filePath);
			return writeStlFile(path, provider);
		}

		// Write STL file directly to disk using a std::filesystem path
		static Result writeStlFile(const std::filesystem::path& filePath, Provider& provider)
		{
			std::ofstream ofs(filePath, std::ios::binary);
			if (!ofs)
				return Result::FileError;
			else
				return writeStlStream(ofs, provider);
		};

		// Write STL file data to a memory buffer
		static Result writeStlBuffer(std::string& buffer, Provider& provider)
		{
			std::ostringstream ss;
			Result result = writeStlStream(ss, provider);
			buffer = std::move(ss.str());
			return result;
		}

		// Parse STL file from a std::istream source
		static Result writeStlStream(std::ostream& os, Provider& provider)
		{
			bool asciiMode = provider.asciiMode();
			Result result = asciiMode ? writeAsciiStream(os, provider) : writeBinaryStream(os, provider);
			return result;
		}

	private:
		static inline const char* libraryName = "microstl";

		static bool isLittleEndian()
		{
			int16_t number = 1;
			const char* ptr = reinterpret_cast<const char*>(&number);
			return ptr[0] == 1;
		}

		static Result writeAsciiStream(std::ostream& os, Provider& provider)
		{
			os << "solid";
			std::string name = provider.getName();
			if (!name.empty())
				os << " " << name;
			os << "\n";

			size_t facetCount = provider.getFacetCount();
			bool nullifyNormals = provider.nullifyNormals();
			for (size_t i = 0; i < facetCount; ++i)
			{
				float n[3] = { 0, };
				float v[9] = { 0, };
				provider.getFacet(i, v + 0, v + 3, v + 6, n);
				if (nullifyNormals)
					os << "  facet normal 0 0 0\n";
				else
					os << "  facet normal " << n[0] << " " << n[1] << " " << n[2] << "\n";
				os << "    outer loop\n";
				os << "      vertex " << v[0] << " " << v[1] << " " << v[2] << "\n";
				os << "      vertex " << v[3] << " " << v[4] << " " << v[5] << "\n";
				os << "      vertex " << v[6] << " " << v[7] << " " << v[8] << "\n";
				os << "    endloop\n";
				os << "  endfacet\n";
			}
			os << "endsolid\n";
			return Result::Success;
		}

		static Result writeBinaryStream(std::ostream& os, Provider& provider)
		{
			if (!isLittleEndian())
				return Result::EndianError;

			uint8_t buffer[80];
			provider.getHeader(buffer);
			os.write(reinterpret_cast<const char*>(buffer), sizeof(buffer));

			size_t facetCount = provider.getFacetCount();
			uint32_t tmp = static_cast<uint32_t>(facetCount);
			os.write(reinterpret_cast<const char*>(&tmp), 4);

			float nullNormals[3] = { 0 ,0,0 };
			bool nullifyNormals = provider.nullifyNormals();

			bool writeAttributes = provider.writeAttributes();

			for (size_t i = 0; i < facetCount; ++i)
			{
				float n[3] = { 0, };
				float v[9] = { 0, };
				provider.getFacet(i, v + 0, v + 3, v + 6, n);
				if (nullifyNormals)
					os.write(reinterpret_cast<const char*>(nullNormals), 3 * sizeof(float));
				else
					os.write(reinterpret_cast<const char*>(n), 3 * sizeof(float));
				os.write(reinterpret_cast<const char*>(v), 9 * sizeof(float));

				uint8_t a[2] = { 0, 0 };
				if (writeAttributes)
					provider.getFacetAttributes(i, a);
				os.write(reinterpret_cast<const char*>(a), 2);
			}

			return Result::Success;
		}
	};

	// Converts the result enum values to readable strings
	std::string getResultString(Result result)
	{
		if (result >= Result::__LAST__RESULT__VALUE)
			throw std::runtime_error("Invalid result value!");

		static_assert(sizeof(Result) == sizeof(uint16_t), "Please adjust the code below with new type!");
		const uint16_t knowLastValue = 9u;
		const uint16_t currentLastValue = static_cast<uint16_t>(Result::__LAST__RESULT__VALUE);
		static_assert(knowLastValue == currentLastValue, "Please extend the switch cases!");
		switch (result)
		{
		case microstl::Result::Undefined:
			return "Undefined";
		case microstl::Result::Success:
			return "Success";
		case microstl::Result::FileError:
			return "FileError";
		case microstl::Result::MissingDataError:
			return "MissingDataError";
		case microstl::Result::UnexpectedError:
			return "UnexpectedError";
		case microstl::Result::ParserError:
			return "ParserError";
		case microstl::Result::LineLimitError:
			return "LineLimitError";
		case microstl::Result::FacetCountError:
			return "FacetCountError";
		case microstl::Result::EndianError:
			return "EndianError";
		default:
			throw std::runtime_error("Invalid result value!");
		}
	}

	// Simple data structures for meshes
	struct Normal { float x, y, z; };
	struct Vertex { float x, y, z; };

	// Each facet contains a copy of all three vertex coordinates
	struct Facet { Vertex v1; Vertex v2; Vertex v3; Normal n; };
	struct Mesh { std::vector<Facet> facets; };

	// Each facet has three vertex indices
	struct FVFacet { size_t v1; size_t v2; size_t v3; Normal n; };
	struct FVMesh { std::vector<Vertex> vertices; std::vector<FVFacet> facets; };

	struct MeshParserHandler : Parser::Handler
	{
		Mesh mesh;
		std::string name;
		std::vector<uint8_t> header;
		bool ascii;
		size_t errorLineNumber;
		microstl::Result result;

		MeshParserHandler() { reset(); }
		void onName(const std::string& n) override { name = n; }
		void onBegin(bool m) override { reset();  ascii = m; }
		void onBinaryHeader(const uint8_t buffer[80]) override { header.resize(80); memcpy(header.data(), buffer, 80); }
		void onError(size_t l) override { errorLineNumber = l; }
		void onEnd(Result r) override { result = r; }

		void reset()
		{
			mesh.facets.clear();
			name.clear();
			header.clear();
			ascii = false;
			errorLineNumber = 0;
			result = microstl::Result::Undefined;
		}

		void onFacet(const float v1[3], const float v2[3], const float v3[3], const float n[3]) override
		{
			Facet facet;
			facet.v1 = { v1[0], v1[1], v1[2] };
			facet.v2 = { v2[0], v2[1], v2[2] };
			facet.v3 = { v3[0], v3[1], v3[2] };
			facet.n = { n[0], n[1], n[2] };
			mesh.facets.push_back(std::move(facet));
		}
	};

	// The mesh provider can be used to write a mesh using the writer
	struct MeshProvider : microstl::Writer::Provider
	{
		const microstl::Mesh& mesh;

		MeshProvider(const microstl::Mesh& m) : mesh(m) {}
		size_t getFacetCount() override { return mesh.facets.size(); }

		void getFacet(size_t index, float v1[3], float v2[3], float v3[3], float n[3]) override
		{
			const auto& facet = mesh.facets[index];
			v1[0] = facet.v1.x; v1[1] = facet.v1.y; v1[2] = facet.v1.z;
			v2[0] = facet.v2.x; v2[1] = facet.v2.y; v2[2] = facet.v2.z;
			v3[0] = facet.v3.x; v3[1] = facet.v3.y; v3[2] = facet.v3.z;
			n[0] = facet.n.x; n[1] = facet.n.y; n[2] = facet.n.z;
		}
	};

	// The FV mesh provider can be used to write face-vertex meshes using the writer
	struct FVMeshProvider : microstl::Writer::Provider
	{
		const microstl::FVMesh& mesh;

		FVMeshProvider(const microstl::FVMesh& m) : mesh(m) {}
		size_t getFacetCount() override { return mesh.facets.size(); }

		void getFacet(size_t index, float v1[3], float v2[3], float v3[3], float n[3]) override
		{
			const auto& facet = mesh.facets[index];
			v1[0] = mesh.vertices[facet.v1].x; v1[1] = mesh.vertices[facet.v1].y; v1[2] = mesh.vertices[facet.v1].z;
			v2[0] = mesh.vertices[facet.v2].x; v2[1] = mesh.vertices[facet.v2].y; v2[2] = mesh.vertices[facet.v2].z;
			v3[0] = mesh.vertices[facet.v3].x; v3[1] = mesh.vertices[facet.v3].y; v3[2] = mesh.vertices[facet.v3].z;
			n[0] = facet.n.x; n[1] = facet.n.y; n[2] = facet.n.z;
		}
	};

	// Deduplicates the vertices to create a more common face-vertex data structure
	FVMesh deduplicateVertices(const Mesh& inputMesh)
	{
		FVMesh outputMesh;
		auto addVertex = [&outputMesh](const Vertex& v)
		{
			for (size_t i = 0; i < outputMesh.vertices.size(); i++)
			{
				if (v.x == outputMesh.vertices[i].x &&
					v.y == outputMesh.vertices[i].y &&
					v.z == outputMesh.vertices[i].z)
				{
					return i;
				}
			}
			size_t index = outputMesh.vertices.size();
			outputMesh.vertices.push_back(v);
			return index;
		};
		for (const auto& f : inputMesh.facets)
		{
			size_t i1 = addVertex(f.v1);
			size_t i2 = addVertex(f.v2);
			size_t i3 = addVertex(f.v3);
			outputMesh.facets.push_back(FVFacet{i1, i2, i3, f.n});
		}
		return std::move(outputMesh);
	}
};
