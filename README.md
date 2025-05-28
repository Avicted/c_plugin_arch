# C plugin architecture POC

This is a proof of concept for a plugin architecture in C using POSIX-specific features, demonstrating how to dynamically load and manage plugins.
Each plugin can be loaded, initialized, run, and cleaned up, allowing for modular and extensible applications.

The plugins perform simulated work. They run in parallel by forking a process for each plugin, simulating a real-world scenario where plugins might perform tasks concurrently.

CTRL+C to stop the execution through the signal handler. The signal handler ensures that all plugins are cleaned up properly before exiting.

### Notes
To be able to handle signals properly the main program needs to be run in a terminal that supports signals (like a Linux terminal). If you run it in an IDE or a non-terminal environment, the signal handling might not work as expected and the program will not execute.

### Build and Run
```bash
chmod +x build.sh
./build.sh

./build/c_plugin_arch_main
```

### Output
```bash
Found 2 plugins:
  - plugin_1
  - plugin_2

Loading plugins...
	Test Plugin 1 initialized
	Successfully init plugin: Test Plugin 1 (initialized).
	Test Plugin 2 initialized
	Successfully init plugin: Test Plugin 2 (initialized).

Running plugins...
[parent] Forked child 180051 for plugin Test Plugin 1
[parent] Forked child 180052 for plugin Test Plugin 2
	Test Plugin 1 running
	Test Plugin 2 running
	Test Plugin 1 working... 1
	Test Plugin 2 working... 1
	Test Plugin 1 working... 2
	Test Plugin 2 working... 2
	Test Plugin 1 working... 3
	Test Plugin 2 working... 3
	Test Plugin 1 working... 4
	Test Plugin 2 working... 4
	Test Plugin 1 working... 5
	Test Plugin 2 working... 5
	Test Plugin 1 working... 6
	Test Plugin 2 working... 6
	Test Plugin 1 working... 7
	Test Plugin 2 working... 7
	Test Plugin 1 working... 8
	Test Plugin 2 working... 8
	Test Plugin 1 working... 9
	Test Plugin 2 working... 9
	Test Plugin 1 working... 10
	Successfully ran plugin: Test Plugin 1 (ran to completion).
	Test Plugin 2 working... 10
	Successfully ran plugin: Test Plugin 2 (ran to completion).
	Plugin plugin_1 finished running
	Plugin plugin_2 finished running

Cleaning up plugins...
	Test Plugin 1 cleaned up
	Successfully cleanup plugin: Test Plugin 1 (terminated).
	Test Plugin 2 cleaned up
	Successfully cleanup plugin: Test Plugin 2 (terminated).
```

### License
MIT License
