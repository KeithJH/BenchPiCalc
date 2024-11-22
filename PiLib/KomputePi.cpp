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
double KomputePi(const std::size_t iterations, const uint32_t deviceIndex)
{
	constexpr std::size_t SHADER_LOCAL_SIZE = 1024;
	constexpr std::size_t MAX_GROUP_SIZE = 65535;

	kp::Manager manager(deviceIndex);

	const std::size_t totalWorkGroupCount = (iterations / SHADER_LOCAL_SIZE) + (iterations % SHADER_LOCAL_SIZE != 0);
	unsigned dimensionCountX;
	unsigned dimensionCountY;

	if (totalWorkGroupCount > MAX_GROUP_SIZE)
	{
		dimensionCountX = MAX_GROUP_SIZE;
		dimensionCountY = static_cast<unsigned>(totalWorkGroupCount / MAX_GROUP_SIZE) + (totalWorkGroupCount % MAX_GROUP_SIZE != 0);
	}
	else
	{
		dimensionCountX = static_cast<unsigned>(totalWorkGroupCount);
		dimensionCountY = 1;
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
		->eval()
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

double KomputePi(const std::size_t iterations)
{
	return KomputePi(iterations, 0);
}
} // namespace PiLib
