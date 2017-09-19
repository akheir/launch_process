### Launch process example

To Compile:
````
mkdir build
cd build
cmake -DHPX_ROOT=[path_to_HPX_root] ..
make
````

To Run:
````
launch_process_test --launch=[full_path_to_launched_process_test]
launch_process_parallel_test --launch=[full_path_to_launched_process_test]
````