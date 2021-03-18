#include <cstdint>
#include <fstream>
#include <filesystem>

namespace microstl
{
	class IReceiver
	{
	public:
		virtual ~IReceiver() {}
		virtual void receiveName(const std::string& name) = 0;
		virtual void receiveFace(const double v1[3], const double v2[3], const double v3[3]) = 0;
	};

	enum class Result : uint16_t {
		Okay = 0,
		NotImplementedError = 2,
		FileReadError = 3,
		UnexpectedFileEnd = 4,
		InvalidFile = 5,
	};

	bool isAsciiFormat(std::ifstream& ifs)
	{
		char header[5] = { 0, };
		ifs.read(header, sizeof(header));
		ifs.seekg(0, std::ios::beg);
		return memcmp("solid", header, sizeof(header)) == 0;
	}

	bool readNextLine(std::ifstream& ifs, std::string& output)
	{
		output.resize(0);
		if (!ifs.is_open() || !ifs.good() || ifs.eof())
			return false;

		while (!ifs.eof())
		{
			char byte;
			ifs.read(&byte, 1);
			if (byte == '\n')
				return true;
			else
				output.push_back(byte);
		}

		return true;
	}

	std::string stringTrim(const std::string& line)
	{
		std::string result;
		int32_t index = 0;
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

	bool stringStartsWith(const std::string& str, const char* prefix)
	{
		return memcmp(prefix, str.data(), strlen(prefix)) == 0;
	}

	Result parseAsciiStream(std::ifstream& ifs, IReceiver& receiver)
	{
		// State machine variables
		bool activeSolid = false;
		bool activeFacet = false;
		bool activeLoop = false;
		size_t solidCount = 0, facetCount = 0, loopCount = 0, vertexCount = 0;

		// Line parse with loop to work the state machine
		while (true)
		{
			std::string line;
			if (!readNextLine(ifs, line))
				break;
			line = stringTrim(line);
			if (stringStartsWith(line, "solid"))
			{
				if (activeSolid || solidCount != 0)
					return Result::InvalidFile;
				activeSolid = true;
				if (line.length() > 5)
				{
					std::string name = stringTrim(line.substr(5));
					receiver.receiveName(name);
				}
			}
			if (stringStartsWith(line, "endsolid"))
			{
				if (!activeSolid || activeFacet || activeLoop)
					return Result::InvalidFile;
				activeSolid = false;
				solidCount++;
			}
			if (stringStartsWith(line, "facet normal"))
			{
				if (!activeSolid || activeLoop || activeFacet)
					return Result::InvalidFile;
				activeFacet = true;
			}
			if (stringStartsWith(line, "endfacet"))
			{
				if (!activeSolid || activeLoop || !activeFacet || loopCount != 1)
					return Result::InvalidFile;
				activeFacet = false;
				facetCount++;
				loopCount = 0;
			}
			if (stringStartsWith(line, "outer loop"))
			{
				if (!activeSolid || !activeFacet || activeLoop)
					return Result::InvalidFile;
				activeLoop = true;
			}
			if (stringStartsWith(line, "endloop"))
			{
				if (!activeSolid || !activeFacet || !activeLoop || vertexCount != 3)
					return Result::InvalidFile;
				activeLoop = false;
				loopCount++;
				vertexCount = 0;
			}
			if (stringStartsWith(line, "vertex"))
			{
				if (!activeSolid || !activeFacet || !activeLoop)
					return Result::InvalidFile;
				vertexCount++;
			}
		}

		if (activeSolid || activeFacet || activeLoop || solidCount != 1)
			return Result::InvalidFile;

		return Result::Okay;
	}

	Result parseStlStream(std::ifstream& ifs, IReceiver& receiver)
	{
		if (isAsciiFormat(ifs))
			return parseAsciiStream(ifs, receiver);
		else
			return Result::NotImplementedError;
	}

	Result parseStlFile(std::filesystem::path& filePath, IReceiver& receiver)
	{
		std::ifstream ifs(filePath);
		if (!ifs.is_open())
			return Result::FileReadError;

		return parseStlStream(ifs, receiver);
	};
};
