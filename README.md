# C plugin architecture POC

This is a proof of concept for a plugin architecture in C, demonstrating how to dynamically load and manage plugins.
Each plugin can be loaded, initialized, run, and cleaned up, allowing for modular and extensible applications.

The plugins perform simulated work. They run in parallel, and each plugin can be independently loaded and executed.

### Build and Run
```bash
chmod +x build.sh
./build.sh

./build/main
```

### Output
```bash
Found 2 plugins:
  - plugin_1
  - plugin_2

Loading plugins...
        Test Plugin 1 initialized
        Successfully Loaded and initialized plugin: Test Plugin 1

        Test Plugin 2 initialized
        Successfully Loaded and initialized plugin: Test Plugin 2


Running plugins...
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
        Successfully ran plugin: Test Plugin 1

        Test Plugin 2 working... 5
        Successfully ran plugin: Test Plugin 2


Cleaning up plugins...
        Test Plugin 1 cleaned up
        Successfully Cleaned up plugin: Test Plugin 1

        Test Plugin 2 cleaned up
        Successfully Cleaned up plugin: Test Plugin 2
```

### License
MIT License
