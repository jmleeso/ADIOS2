/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#include "TestSscCommon.h"
#include <adios2.h>
#include <gtest/gtest.h>
#include <mpi.h>
#include <numeric>
#include <thread>

using namespace adios2;
int mpiRank = 0;
int mpiSize = 1;
MPI_Comm mpiComm;

char runMode;

class SscEngineTest : public ::testing::Test
{
public:
    SscEngineTest() = default;
};

void Writer(const Dims &shape, const Dims &start, const Dims &count,
            const size_t steps, const adios2::Params &engineParams,
            const std::string &name)
{
    size_t datasize =
        std::accumulate(count.begin(), count.end(), static_cast<size_t>(1),
                        std::multiplies<size_t>());
    adios2::ADIOS adios(mpiComm);
    adios2::IO dataManIO = adios.DeclareIO("WAN");
    dataManIO.SetEngine("ssc");
    dataManIO.SetParameters(engineParams);
    std::vector<char> myChars(datasize);
    std::vector<unsigned char> myUChars(datasize);
    std::vector<short> myShorts(datasize);
    std::vector<unsigned short> myUShorts(datasize);
    std::vector<int> myInts(datasize);
    std::vector<unsigned int> myUInts(datasize);
    std::vector<float> myFloats(datasize);
    std::vector<double> myDoubles(datasize);
    std::vector<std::complex<float>> myComplexes(datasize);
    std::vector<std::complex<double>> myDComplexes(datasize);
    auto bpChars =
        dataManIO.DefineVariable<char>("bpChars", shape, start, count);
    auto bpUChars = dataManIO.DefineVariable<unsigned char>("bpUChars", shape,
                                                            start, count);
    auto bpShorts =
        dataManIO.DefineVariable<short>("bpShorts", shape, start, count);
    auto bpUShorts = dataManIO.DefineVariable<unsigned short>(
        "bpUShorts", shape, start, count);
    auto bpInts = dataManIO.DefineVariable<int>("bpInts", shape, start, count);
    auto bpUInts =
        dataManIO.DefineVariable<unsigned int>("bpUInts", shape, start, count);
    auto bpFloats =
        dataManIO.DefineVariable<float>("bpFloats", shape, start, count);
    auto bpDoubles =
        dataManIO.DefineVariable<double>("bpDoubles", shape, start, count);
    auto bpComplexes = dataManIO.DefineVariable<std::complex<float>>(
        "bpComplexes", shape, start, count);
    auto bpDComplexes = dataManIO.DefineVariable<std::complex<double>>(
        "bpDComplexes", shape, start, count);
    dataManIO.DefineAttribute<int>("AttInt", 110);
    adios2::Engine engine = dataManIO.Open(name, adios2::Mode::Write);
    engine.LockWriterDefinitions();
    for (size_t i = 0; i < steps; ++i)
    {
        engine.BeginStep();
        GenData(myChars, i, start, count, shape);
        GenData(myUChars, i, start, count, shape);
        GenData(myShorts, i, start, count, shape);
        GenData(myUShorts, i, start, count, shape);
        GenData(myInts, i, start, count, shape);
        GenData(myUInts, i, start, count, shape);
        GenData(myFloats, i, start, count, shape);
        GenData(myDoubles, i, start, count, shape);
        GenData(myComplexes, i, start, count, shape);
        GenData(myDComplexes, i, start, count, shape);
        engine.Put(bpChars, myChars.data(), adios2::Mode::Sync);
        engine.Put(bpUChars, myUChars.data(), adios2::Mode::Sync);
        engine.Put(bpShorts, myShorts.data(), adios2::Mode::Sync);
        engine.Put(bpUShorts, myUShorts.data(), adios2::Mode::Sync);
        engine.Put(bpInts, myInts.data(), adios2::Mode::Sync);
        engine.Put(bpUInts, myUInts.data(), adios2::Mode::Sync);
        engine.Put(bpFloats, myFloats.data(), adios2::Mode::Sync);
        engine.Put(bpDoubles, myDoubles.data(), adios2::Mode::Sync);
        engine.Put(bpComplexes, myComplexes.data(), adios2::Mode::Sync);
        engine.Put(bpDComplexes, myDComplexes.data(), adios2::Mode::Sync);
        engine.EndStep();
    }
    engine.Close();
}

void Reader(const Dims &shape, const Dims &start, const Dims &count,
            const size_t steps, const adios2::Params &engineParams,
            const std::string &name)
{
    adios2::ADIOS adios(mpiComm);
    adios2::IO dataManIO = adios.DeclareIO("Test");
    dataManIO.SetEngine("ssc");
    dataManIO.SetParameters(engineParams);
    adios2::Engine engine = dataManIO.Open(name, adios2::Mode::Read);
    engine.LockReaderSelections();

    size_t datasize =
        std::accumulate(shape.begin(), shape.end(), static_cast<size_t>(1),
                        std::multiplies<size_t>());
    std::vector<char> myChars(datasize);
    std::vector<unsigned char> myUChars(datasize);
    std::vector<short> myShorts(datasize);
    std::vector<unsigned short> myUShorts(datasize);
    std::vector<int> myInts(datasize);
    std::vector<unsigned int> myUInts(datasize);
    std::vector<float> myFloats(datasize);
    std::vector<double> myDoubles(datasize);
    std::vector<std::complex<float>> myComplexes(datasize);
    std::vector<std::complex<double>> myDComplexes(datasize);

    while (true)
    {
        adios2::StepStatus status = engine.BeginStep(StepMode::Read, 5);
        if (status == adios2::StepStatus::OK)
        {
            const auto &vars = dataManIO.AvailableVariables();
            ASSERT_EQ(vars.size(), 10);
            size_t currentStep = engine.CurrentStep();
            adios2::Variable<char> bpChars =
                dataManIO.InquireVariable<char>("bpChars");
            adios2::Variable<unsigned char> bpUChars =
                dataManIO.InquireVariable<unsigned char>("bpUChars");
            adios2::Variable<short> bpShorts =
                dataManIO.InquireVariable<short>("bpShorts");
            adios2::Variable<unsigned short> bpUShorts =
                dataManIO.InquireVariable<unsigned short>("bpUShorts");
            adios2::Variable<int> bpInts =
                dataManIO.InquireVariable<int>("bpInts");
            adios2::Variable<unsigned int> bpUInts =
                dataManIO.InquireVariable<unsigned int>("bpUInts");
            adios2::Variable<float> bpFloats =
                dataManIO.InquireVariable<float>("bpFloats");
            adios2::Variable<double> bpDoubles =
                dataManIO.InquireVariable<double>("bpDoubles");
            adios2::Variable<std::complex<float>> bpComplexes =
                dataManIO.InquireVariable<std::complex<float>>("bpComplexes");
            adios2::Variable<std::complex<double>> bpDComplexes =
                dataManIO.InquireVariable<std::complex<double>>("bpDComplexes");

            engine.Get(bpChars, myChars.data(), adios2::Mode::Sync);
            engine.Get(bpUChars, myUChars.data(), adios2::Mode::Sync);
            engine.Get(bpShorts, myShorts.data(), adios2::Mode::Sync);
            engine.Get(bpUShorts, myUShorts.data(), adios2::Mode::Sync);
            engine.Get(bpInts, myInts.data(), adios2::Mode::Sync);
            engine.Get(bpUInts, myUInts.data(), adios2::Mode::Sync);
            engine.Get(bpFloats, myFloats.data(), adios2::Mode::Sync);
            engine.Get(bpDoubles, myDoubles.data(), adios2::Mode::Sync);
            engine.Get(bpComplexes, myComplexes.data(), adios2::Mode::Sync);
            engine.Get(bpDComplexes, myDComplexes.data(), adios2::Mode::Sync);
            VerifyData(myChars.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            VerifyData(myUChars.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            VerifyData(myShorts.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            VerifyData(myUShorts.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            VerifyData(myInts.data(), currentStep, Dims(shape.size(), 0), shape,
                       shape, mpiRank);
            VerifyData(myUInts.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            VerifyData(myFloats.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            VerifyData(myDoubles.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            VerifyData(myComplexes.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            VerifyData(myDComplexes.data(), currentStep, Dims(shape.size(), 0),
                       shape, shape, mpiRank);
            engine.EndStep();
        }
        else if (status == adios2::StepStatus::EndOfStream)
        {
            std::cout << "[Rank " + std::to_string(mpiRank) +
                             "] SscTest reader end of stream!"
                      << std::endl;
            break;
        }
    }
    engine.Close();
}

TEST_F(SscEngineTest, TestSscMoreReadersThanWriters)
{
    {
        std::string filename = "TestSscMoreReadersThanWriters";
        adios2::Params engineParams = {};

        int worldRank, worldSize;
        Dims start, count, shape;
        MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
        MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
        int mpiGroup;
        int writers = 2;
        if (worldSize < 3)
        {
            return;
        }
        if (worldRank < writers)
        {
            mpiGroup = 0;
        }
        else
        {
            mpiGroup = 1;
        }

        MPI_Comm_split(MPI_COMM_WORLD, mpiGroup, worldRank, &mpiComm);

        MPI_Comm_rank(mpiComm, &mpiRank);
        MPI_Comm_size(mpiComm, &mpiSize);

        size_t steps = 20;

        if (mpiGroup == 0)
        {
            shape = {(size_t)writers, 10};
            start = {(size_t)mpiRank, 0};
            count = {1, 10};
            Writer(shape, start, count, steps, engineParams, filename);
        }

        if (mpiGroup == 1)
        {
            shape = {(size_t)writers, 10};
            start = {0, 0};
            count = shape;
            Reader(shape, start, shape, steps, engineParams, filename);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    {
        std::string filename = "TestSscMoreReadersThanWritersNaive";
        adios2::Params engineParams = {{"Verbose", "0"},
                                       {"EngineMode", "naive"}};

        int worldRank, worldSize;
        Dims start, count, shape;
        MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
        MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
        int mpiGroup;
        int writers = 2;
        if (worldSize < 3)
        {
            return;
        }
        if (worldRank < writers)
        {
            mpiGroup = 0;
        }
        else
        {
            mpiGroup = 1;
        }

        MPI_Comm_split(MPI_COMM_WORLD, mpiGroup, worldRank, &mpiComm);

        MPI_Comm_rank(mpiComm, &mpiRank);
        MPI_Comm_size(mpiComm, &mpiSize);

        size_t steps = 20;

        if (mpiGroup == 0)
        {
            shape = {(size_t)writers, 10};
            start = {(size_t)mpiRank, 0};
            count = {1, 10};
            Writer(shape, start, count, steps, engineParams, filename);
        }

        if (mpiGroup == 1)
        {
            shape = {(size_t)writers, 10};
            start = {0, 0};
            count = shape;
            Reader(shape, start, shape, steps, engineParams, filename);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int worldRank, worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    MPI_Finalize();
    return result;
}
