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
        std::accumulate(shape.begin(), shape.end(), static_cast<size_t>(1),
                        std::multiplies<size_t>());
    adios2::ADIOS adios(mpiComm);
    adios2::IO io = adios.DeclareIO("Test");
    io.SetEngine("ssc");
    io.SetParameters(engineParams);
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
    auto bpChars = io.DefineVariable<char>("bpChars", shape, start, shape);
    auto bpUChars =
        io.DefineVariable<unsigned char>("bpUChars", shape, start, shape);
    auto bpShorts = io.DefineVariable<short>("bpShorts", shape, start, shape);
    auto bpUShorts =
        io.DefineVariable<unsigned short>("bpUShorts", shape, start, shape);
    auto bpInts = io.DefineVariable<int>("bpInts", shape, start, shape);
    auto bpUInts =
        io.DefineVariable<unsigned int>("bpUInts", shape, start, shape);
    auto bpFloats = io.DefineVariable<float>("bpFloats", shape, start, shape);
    auto bpDoubles =
        io.DefineVariable<double>("bpDoubles", shape, start, shape);
    auto bpComplexes = io.DefineVariable<std::complex<float>>(
        "bpComplexes", shape, start, shape);
    auto bpDComplexes = io.DefineVariable<std::complex<double>>(
        "bpDComplexes", shape, start, shape);
    auto scalarInt = io.DefineVariable<int>("scalarInt");
    auto stringVar = io.DefineVariable<std::string>("stringVar");
    io.DefineAttribute<int>("AttInt", 110);
    adios2::Engine engine = io.Open(name, adios2::Mode::Write);
    engine.LockWriterDefinitions();
    for (size_t i = 0; i < steps; ++i)
    {
        engine.BeginStep();
        if (mpiRank == 0)
        {
            GenData(myChars, i, start, shape, shape);
            GenData(myUChars, i, start, shape, shape);
            GenData(myShorts, i, start, shape, shape);
            GenData(myUShorts, i, start, shape, shape);
            GenData(myInts, i, start, shape, shape);
            GenData(myUInts, i, start, shape, shape);
            GenData(myFloats, i, start, shape, shape);
            GenData(myDoubles, i, start, shape, shape);
            GenData(myComplexes, i, start, shape, shape);
            GenData(myDComplexes, i, start, shape, shape);
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
            engine.Put(scalarInt, static_cast<int>(i));
            std::string s = "sample string sample string sample string";
            engine.Put(stringVar, s);
        }
        engine.EndStep();
    }
    engine.Close();
}

void Reader(const Dims &shape, const Dims &start, const Dims &count,
            const size_t steps, const adios2::Params &engineParams,
            const std::string &name)
{
    adios2::ADIOS adios(mpiComm);
    adios2::IO io = adios.DeclareIO("Test");
    io.SetEngine("ssc");
    io.SetParameters(engineParams);
    adios2::Engine engine = io.Open(name, adios2::Mode::Read);

    size_t datasize =
        std::accumulate(count.begin(), count.end(), static_cast<size_t>(1),
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

    engine.LockReaderSelections();

    while (true)
    {
        adios2::StepStatus status = engine.BeginStep(StepMode::Read, 5);
        if (status == adios2::StepStatus::OK)
        {
            auto scalarInt = io.InquireVariable<int>("scalarInt");
            auto blocksInfo =
                engine.BlocksInfo(scalarInt, engine.CurrentStep());

            for (const auto &bi : blocksInfo)
            {
                ASSERT_EQ(bi.IsValue, true);
                ASSERT_EQ(bi.Value, engine.CurrentStep());
                ASSERT_EQ(scalarInt.Min(), engine.CurrentStep());
                ASSERT_EQ(scalarInt.Max(), engine.CurrentStep());
            }

            const auto &vars = io.AvailableVariables();
            ASSERT_EQ(vars.size(), 12);
            size_t currentStep = engine.CurrentStep();
            adios2::Variable<char> bpChars =
                io.InquireVariable<char>("bpChars");
            adios2::Variable<unsigned char> bpUChars =
                io.InquireVariable<unsigned char>("bpUChars");
            adios2::Variable<short> bpShorts =
                io.InquireVariable<short>("bpShorts");
            adios2::Variable<unsigned short> bpUShorts =
                io.InquireVariable<unsigned short>("bpUShorts");
            adios2::Variable<int> bpInts = io.InquireVariable<int>("bpInts");
            adios2::Variable<unsigned int> bpUInts =
                io.InquireVariable<unsigned int>("bpUInts");
            adios2::Variable<float> bpFloats =
                io.InquireVariable<float>("bpFloats");
            adios2::Variable<double> bpDoubles =
                io.InquireVariable<double>("bpDoubles");
            adios2::Variable<std::complex<float>> bpComplexes =
                io.InquireVariable<std::complex<float>>("bpComplexes");
            adios2::Variable<std::complex<double>> bpDComplexes =
                io.InquireVariable<std::complex<double>>("bpDComplexes");
            adios2::Variable<std::string> stringVar =
                io.InquireVariable<std::string>("stringVar");

            bpChars.SetSelection({start, count});
            bpUChars.SetSelection({start, count});
            bpShorts.SetSelection({start, count});
            bpUShorts.SetSelection({start, count});
            bpInts.SetSelection({start, count});
            bpUInts.SetSelection({start, count});
            bpFloats.SetSelection({start, count});
            bpDoubles.SetSelection({start, count});
            bpComplexes.SetSelection({start, count});
            bpDComplexes.SetSelection({start, count});

            engine.Get(bpChars, myChars.data(), adios2::Mode::Sync);
            engine.Get(bpUChars, myUChars.data(), adios2::Mode::Sync);
            engine.Get(bpShorts, myShorts.data(), adios2::Mode::Sync);
            engine.Get(bpUShorts, myUShorts.data(), adios2::Mode::Sync);
            engine.Get(bpInts, myInts.data(), adios2::Mode::Sync);
            engine.Get(bpUInts, myUInts.data(), adios2::Mode::Sync);

            VerifyData(myChars.data(), currentStep, start, count, shape,
                       mpiRank);
            VerifyData(myUChars.data(), currentStep, start, count, shape,
                       mpiRank);
            VerifyData(myShorts.data(), currentStep, start, count, shape,
                       mpiRank);
            VerifyData(myUShorts.data(), currentStep, start, count, shape,
                       mpiRank);
            VerifyData(myInts.data(), currentStep, start, count, shape,
                       mpiRank);
            VerifyData(myUInts.data(), currentStep, start, count, shape,
                       mpiRank);

            engine.Get(bpFloats, myFloats.data(), adios2::Mode::Deferred);
            engine.Get(bpDoubles, myDoubles.data(), adios2::Mode::Deferred);
            engine.Get(bpComplexes, myComplexes.data(), adios2::Mode::Deferred);
            engine.Get(bpDComplexes, myDComplexes.data(),
                       adios2::Mode::Deferred);
            engine.PerformGets();

            VerifyData(myFloats.data(), currentStep, start, count, shape,
                       mpiRank);
            VerifyData(myDoubles.data(), currentStep, start, count, shape,
                       mpiRank);
            VerifyData(myComplexes.data(), currentStep, start, count, shape,
                       mpiRank);
            VerifyData(myDComplexes.data(), currentStep, start, count, shape,
                       mpiRank);

            std::string s;
            engine.Get(stringVar, s);
            engine.PerformGets();
            ASSERT_EQ(s, "sample string sample string sample string");
            ASSERT_EQ(stringVar.Min(),
                      "sample string sample string sample string");
            ASSERT_EQ(stringVar.Max(),
                      "sample string sample string sample string");

            int i;
            engine.Get(scalarInt, &i);
            engine.PerformGets();
            ASSERT_EQ(i, currentStep);
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
    auto attInt = io.InquireAttribute<int>("AttInt");
    std::cout << "[Rank " + std::to_string(mpiRank) + "] Attribute received "
              << attInt.Data()[0] << ", expected 110" << std::endl;
    ASSERT_EQ(110, attInt.Data()[0]);
    ASSERT_NE(111, attInt.Data()[0]);
    engine.Close();
}

TEST_F(SscEngineTest, TestSscZeroBlock)
{
    {
        std::string filename = "TestSscZeroBlock";
        adios2::Params engineParams = {{"Verbose", "0"}};
        int worldRank, worldSize;
        MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
        MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
        int mpiGroup = worldRank / (worldSize / 2);
        MPI_Comm_split(MPI_COMM_WORLD, mpiGroup, worldRank, &mpiComm);
        MPI_Comm_rank(mpiComm, &mpiRank);
        MPI_Comm_size(mpiComm, &mpiSize);
        Dims shape = {10, (size_t)mpiSize * 2};
        Dims start = {2, (size_t)mpiRank * 2};
        Dims count = {5, 2};
        size_t steps = 100;
        if (mpiGroup == 0)
        {
            Writer(shape, start, count, steps, engineParams, filename);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (mpiGroup == 1)
        {
            Reader(shape, start, count, steps, engineParams, filename);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    {
        std::string filename = "TestSscZeroBlock";
        adios2::Params engineParams = {{"Verbose", "0"},
                                       {"EngineMode", "naive"}};
        int worldRank, worldSize;
        MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
        MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
        int mpiGroup = worldRank / (worldSize / 2);
        MPI_Comm_split(MPI_COMM_WORLD, mpiGroup, worldRank, &mpiComm);
        MPI_Comm_rank(mpiComm, &mpiRank);
        MPI_Comm_size(mpiComm, &mpiSize);
        Dims shape = {10, (size_t)mpiSize * 2};
        Dims start = {2, (size_t)mpiRank * 2};
        Dims count = {5, 2};
        size_t steps = 100;
        if (mpiGroup == 0)
        {
            Writer(shape, start, count, steps, engineParams, filename);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (mpiGroup == 1)
        {
            Reader(shape, start, count, steps, engineParams, filename);
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
