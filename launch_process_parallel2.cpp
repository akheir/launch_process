//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/process.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <components/launch_process_test_server.hpp>

#include <boost/filesystem.hpp>

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
inline int get_arraylen(char** arr)
{
    int count = 0;
    if (nullptr != arr)
    {
        while (nullptr != arr[count])
            ++count;    // simply count the strings
    }
    return count;
}

std::vector<std::string> get_environment()
{
    std::vector<std::string> env;
#if defined(HPX_WINDOWS)
    int len = get_arraylen(_environ);
    std::copy(&_environ[0], &_environ[len], std::back_inserter(env));
#elif defined(linux) || defined(__linux) || defined(__linux__) ||              \
    defined(__AIX__) || defined(__APPLE__) || defined(__FreeBSD__)
    int len = get_arraylen(environ);
    std::copy(&environ[0], &environ[len], std::back_inserter(env));
#else
#error "Don't know, how to access the execution environment on this platform"
#endif
    return env;
}

namespace process = hpx::components::process;
namespace fs = boost::filesystem;

process::child launch_proc(int proc_number, fs::path exe, fs::path base_dir,
    std::vector<std::string> env)
{
    // set up command line for launched executable
    std::vector<std::string> args;
    args.push_back(exe.string());
    args.push_back("--exit_code=42");
    args.push_back("--component=test_server" + std::to_string(proc_number));
    args.push_back("--set_message=accessed");
    args.push_back("--hpx:ignore-batch-env");

    //    // set up environment for launched executable
    //    std::vector<std::string> env = get_environment();    // current environment

    // Pass along the console parcelport address
    env.push_back("HPX_AGAS_SERVER_ADDRESS=" +
        hpx::get_config_entry("hpx.agas.address", HPX_INITIAL_IP_ADDRESS));
    env.push_back("HPX_AGAS_SERVER_PORT=" +
        hpx::get_config_entry(
            "hpx.agas.port", std::to_string(HPX_INITIAL_IP_PORT)));

    // Pass along the parcelport address which should be used by the launched
    // executable

    // The launched executable will run on the same host as this test
    int port = 42;    // each launched HPX locality needs to be assigned a
    // unique port

    env.push_back("HPX_PARCEL_SERVER_ADDRESS=" +
        hpx::get_config_entry("hpx.agas.address", HPX_INITIAL_IP_ADDRESS));
    env.push_back("HPX_PARCEL_SERVER_PORT=" +
        std::to_string(HPX_CONNECTING_IP_PORT - port - proc_number));

    // Instruct new locality to connect back on startup using the given name.
    env.push_back("HPX_ON_STARTUP_WAIT_ON_LATCH=launch_process" +
        std::to_string(proc_number));

    // launch test executable
    process::child c = process::execute(hpx::find_here(),
        process::run_exe(exe.string()),
        process::set_args(args),
        process::set_env(env),
        process::start_in_dir(base_dir.string()),
        process::throw_on_error(),
        process::wait_on_latch(
            "launch_process" + std::to_string(proc_number))    // same as above!
    );

    c.wait();
    HPX_TEST(c);
    std::cout << "process " << proc_number << " created \n" << std::endl;
    return c;
}

hpx::components::client<launch_process::test_server> create_job(int job_number)
{
    // now create an instance of the test_server component
    hpx::components::client<launch_process::test_server> t =
        hpx::new_<launch_process::test_server>(hpx::find_here());

    hpx::future<std::string> f =
        hpx::async(launch_process_get_message_action(), t);
    HPX_TEST_EQ(f.get(), std::string("initialized"));

    // register the component instance with AGAS
    t.register_as("test_server" +
        std::to_string(job_number));    // same as --component=<> above
    std::cout << "component " << job_number << " created \n" << std::endl;

    return t;
}

// ----------------------------------------------------------------------------
int hpx_main(boost::program_options::variables_map& vm)
{
    // find where the HPX core libraries are located
    fs::path base_dir = hpx::util::find_prefix();
    base_dir /= "bin";

    fs::path exe = base_dir / "launched_process_test" HPX_EXECUTABLE_EXTENSION;

    std::string launch_target;
    if (vm.count("launch"))
    {
        launch_target = vm["launch"].as<std::string>();
        std::cout << "using launch: " << launch_target << std::endl;
        exe = launch_target;
    }
    else
    {
        std::cout << "using launch (default): " << exe << std::endl;
    }

    // set up environment for launched executable
    std::vector<std::string> env = get_environment();    // current environment

    // create the first process
    process::child c0 = launch_proc(0, exe, base_dir, env);

    // attach second process to first one
    hpx::future<int> r0 = c0.wait_for_exit();
    hpx::future<process::child> c1 =
        r0.then([exe, base_dir, env](hpx::future<int> r) {
            return launch_proc(1, exe, base_dir, env);
        });

    // attach third process to second one
    hpx::future<int> r1 =
        c1.then([exe, base_dir, env](hpx::future<process::child> c) {
            return c.get().wait_for_exit();
        });
    hpx::future<process::child> c2 =
        r1.then([exe, base_dir, env](hpx::future<int> r) {
            return launch_proc(2, exe, base_dir, env);
        });

    typedef hpx::components::client<launch_process::test_server> job_type;

    //    job_type t0 = create_job(0);
    //    job_type t1 = create_job(1);
    //    job_type t2 = create_job(2);
    //
    //
    std::vector<job_type> jobs;
    //    jobs.push_back(t0);
    //    jobs.push_back(t1);
    //    jobs.push_back(t2);

    for (int i = 0; i < 3; i++)
        jobs.push_back(create_job(i));

    hpx::future<int> exit_code = c2.get().wait_for_exit();
    HPX_TEST_EQ(exit_code.get(), 42);

    //    hpx::future<std::string> f0 =
    //        hpx::async(launch_process_get_message_action(), t0);
    //    HPX_TEST_EQ(f0.get(), std::string("accessed"));
    //    hpx::future<std::string> f1 =
    //        hpx::async(launch_process_get_message_action(), t1);
    //    HPX_TEST_EQ(f1.get(), std::string("accessed"));
    //    hpx::future<std::string> f2 =
    //        hpx::async(launch_process_get_message_action(), t2);
    //    HPX_TEST_EQ(f2.get(), std::string("accessed"));

    for (auto t : jobs)
    {
        hpx::future<std::string> f0 =
            hpx::async(launch_process_get_message_action(), t);
        HPX_TEST_EQ(f0.get(), std::string("accessed"));
    }

    return hpx::finalize();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace boost::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("launch,l", value<std::string>(),
        "the process that will be launched and which connects back");

    // This explicitly enables the component we depend on (it is disabled by
    // default to avoid being loaded outside of this test).
    std::vector<std::string> const cfg = {
        "hpx.components.launch_process_test_server.enabled!=1"};

    HPX_TEST_EQ_MSG(hpx::init(desc_commandline, argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
