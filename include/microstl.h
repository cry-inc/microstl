#include <cstdint>
#include <fstream>
#include <filesystem>

namespace microstl
{
	class IReceiver
	{
	public:
		virtual ~IReceiver() {}
		virtual void receiveFace(double v1[3], double v2[3], double v3[3]) = 0;
	};

	enum class Result : uint16_t {
		Okay = 0,
		NotImplementedError = 2,
		FileReadError = 3,
	};

	Result parseStlStream(std::ifstream& ifs, IReceiver& receiver)
	{
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
