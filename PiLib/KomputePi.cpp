#include <cstdint>
#include <cstdio>
#include <kompute/Algorithm.hpp>
#include <kompute/Core.hpp>
#include <kompute/Kompute.hpp>
#include <kompute/Manager.hpp>

#include "PiLib.h"
#include "shaders/PartialSum.hpp"
#include "shaders/Reduce.hpp"

namespace PiLib
{

// A bit messy, but helps test if multiple dispatch is worth it
class Algo
{
  public:
	Algo(kp::Manager &manager, const kp::Workgroup &workgroupSize, const std::vector<uint32_t> &partialSumSpirv,
	     const std::vector<uint32_t> &reduceSpirv)
	{
		partialSize = {workgroupSize[0] * workgroupSize[1]};
		resultTensor = manager.tensorT<double>(1);
		partialResultTensor = manager.tensorT<double>(partialSize[0], kp::Memory::MemoryTypes::eStorage);

		partialSumAlgo = manager.algorithm({partialResultTensor}, partialSumSpirv, workgroupSize, {}, {0, 0});
		reduceAlgo = manager.algorithm({partialResultTensor, resultTensor}, reduceSpirv, kp::Workgroup({1}), {}, {0});
	}

	void AsyncPartialSum(kp::Manager &manager, const uint32_t offset, const uint32_t offsetSize,
	                     const uint32_t totalDenom)
	{
		assert(nullptr == inProgress);

		std::vector<uint32_t> partialSumPush{totalDenom, offset * offsetSize};
		inProgress = manager.sequence()->evalAsync<kp::OpAlgoDispatch>(partialSumAlgo, partialSumPush);
	}

	double SyncReduce()
	{
		assert(nullptr != inProgress);

		inProgress->evalAwait()
			->record<kp::OpAlgoDispatch>(reduceAlgo, partialSize)
			->record<kp::OpSyncLocal>({resultTensor})
			->eval();

		inProgress = nullptr;

		return resultTensor->data()[0];
	}

  private:
	std::shared_ptr<kp::TensorT<double>> resultTensor;
	std::shared_ptr<kp::TensorT<double>> partialResultTensor;
	std::shared_ptr<kp::Algorithm> partialSumAlgo;
	std::shared_ptr<kp::Algorithm> reduceAlgo;

	std::vector<uint32_t> partialSize;
	std::shared_ptr<kp::Sequence> inProgress;
};

double KomputePi(const std::size_t iterations, const uint32_t deviceIndex)
{
	constexpr std::size_t SHADER_LOCAL_SIZE = 1024;
	constexpr std::size_t MAX_DIMX_SIZE = 65535;
	constexpr std::size_t MAX_DIMY_SIZE = 32;

	kp::Manager manager(deviceIndex);

	const std::size_t totalWorkGroupCount = (iterations / SHADER_LOCAL_SIZE) + (iterations % SHADER_LOCAL_SIZE != 0);
	unsigned dimensionCountX;
	unsigned dimensionCountY;
	unsigned invocationCount = 1;

	if (totalWorkGroupCount > MAX_DIMX_SIZE)
	{
		dimensionCountX = MAX_DIMX_SIZE;
		dimensionCountY =
			static_cast<unsigned>(totalWorkGroupCount / MAX_DIMX_SIZE) + (totalWorkGroupCount % MAX_DIMX_SIZE != 0);

		if (dimensionCountY > MAX_DIMY_SIZE)
		{
			invocationCount =
				static_cast<unsigned>(dimensionCountY / MAX_DIMY_SIZE) + (totalWorkGroupCount % MAX_DIMX_SIZE != 0);
			dimensionCountY = MAX_DIMY_SIZE;
		}
	}
	else
	{
		dimensionCountX = static_cast<unsigned>(totalWorkGroupCount);
		dimensionCountY = 1;
	}

	const kp::Workgroup workgroup{dimensionCountX, dimensionCountY};
	const std::vector<uint32_t> partialSize{dimensionCountX * dimensionCountY};

	const uint32_t offsetSize = dimensionCountX * dimensionCountY * SHADER_LOCAL_SIZE;
	const uint32_t stepDenom = invocationCount * offsetSize;

	const std::vector<uint32_t> partialSumSpirv = {shaders::PARTIALSUM_COMP_SPV.begin(),
	                                               shaders::PARTIALSUM_COMP_SPV.end()};
	const std::vector<uint32_t> reduceSpirv = {shaders::REDUCE_COMP_SPV.begin(), shaders::REDUCE_COMP_SPV.end()};

	Algo kompute[2]{
		Algo(manager, workgroup, partialSumSpirv, reduceSpirv),
		Algo(manager, workgroup, partialSumSpirv, reduceSpirv),
	};

	double pi = 0;
	Algo *running = &kompute[0];
	Algo *next = &kompute[1];

	running->AsyncPartialSum(manager, 0, offsetSize, stepDenom);
	for (unsigned i = 1; i < invocationCount; i++)
	{
		next->AsyncPartialSum(manager, i, offsetSize, stepDenom);
		pi += running->SyncReduce();

		std::swap(next, running);
	}
	pi += running->SyncReduce();

	pi /= dimensionCountX;
	pi /= dimensionCountY;
	pi /= SHADER_LOCAL_SIZE;
	pi /= invocationCount;

	return pi;
}

double KomputePi(const std::size_t iterations)
{
	return KomputePi(iterations, 0);
}
} // namespace PiLib
