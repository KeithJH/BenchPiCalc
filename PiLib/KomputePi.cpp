#include <cstdint>
#include <kompute/Algorithm.hpp>
#include <kompute/Core.hpp>
#include <kompute/Kompute.hpp>
#include <kompute/Manager.hpp>

#include "PiLib.h"
#include "shaders/PartialSum.hpp"
#include "shaders/Reduce.hpp"

namespace PiLib
{
double KomputePi(const int64_t iterations, const int32_t deviceIndex)
{
	constexpr unsigned SHADER_LOCAL_SIZE = 1024;
	constexpr unsigned MAX_GROUP_SIZE = 65535;

	kp::Manager manager(deviceIndex);

	const unsigned totalWorkGroupCount = (iterations / SHADER_LOCAL_SIZE) + (iterations % SHADER_LOCAL_SIZE != 0);
	unsigned dimensionCountX = totalWorkGroupCount;
	unsigned dimensionCountY = 1;

	if (totalWorkGroupCount > MAX_GROUP_SIZE)
	{
		dimensionCountX = MAX_GROUP_SIZE;
		dimensionCountY = (totalWorkGroupCount / MAX_GROUP_SIZE) + (totalWorkGroupCount % MAX_GROUP_SIZE != 0);
	}

	const kp::Workgroup workgroup{dimensionCountX, dimensionCountY};
	const std::vector<uint32_t> partialSize{dimensionCountX * dimensionCountY};

	const std::vector<uint32_t> partialSumSpirv = {shaders::PARTIALSUM_COMP_SPV.begin(),
	                                               shaders::PARTIALSUM_COMP_SPV.end()};
	const std::vector<uint32_t> reduceSpirv = {shaders::REDUCE_COMP_SPV.begin(), shaders::REDUCE_COMP_SPV.end()};

	auto resultTensor = manager.tensorT<double>(1);
	auto partialResultTensor = manager.tensorT<double>(partialSize[0], kp::Memory::MemoryTypes::eStorage);

	auto partialSumAlgo = manager.algorithm({partialResultTensor}, partialSumSpirv, workgroup);
	auto reduceAlgo =
		manager.algorithm({partialResultTensor, resultTensor}, reduceSpirv, kp::Workgroup({1}), {}, {0.0});

	manager.sequence()
		->record<kp::OpAlgoDispatch>(partialSumAlgo)
		->record<kp::OpAlgoDispatch>(reduceAlgo, partialSize)
		->record<kp::OpSyncLocal>({resultTensor})
		->eval();

	double cpuResult = resultTensor->data()[0];
	double pi = cpuResult;
	pi /= dimensionCountX;
	pi /= dimensionCountY;
	pi /= SHADER_LOCAL_SIZE;

	return pi;
}

double KomputePi(const int64_t iterations)
{
	return KomputePi(iterations, 0);
}
} // namespace PiLib
