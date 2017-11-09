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

launch_process_test2 and launch_process_test3 are the new ones that works by attaching 
continuation to processes to create a chain of three process back to back. 
The launch_process_test3 launches 4 of these processes at same time, which all have 
set of processes to follow. 