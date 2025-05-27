# C plugin architecture POC

This is a proof of concept for a plugin architecture in C, demonstrating how to dynamically load and manage plugins.
Each plugin can be loaded, initialized, run, and cleaned up, allowing for modular and extensible applications.

The plugins perform simulated work. They run in parallel, and each plugin can be independently loaded and executed.


### Output
```bash
Found 2 plugins:
  - plugin_1
  - plugin_2

Loading plugins...
Plugin 1 initialized
        Successfully loaded and initialized plugin: plugin_1
Plugin 2 initialized
        Successfully loaded and initialized plugin: plugin_2

Running plugins...
Plugin 1 running
Plugin 2 running
Plugin 1 working... 1
Plugin 2 working... 1
Plugin 1 working... 2
Plugin 2 working... 2
Plugin 1 working... 3
Plugin 2 working... 3
Plugin 1 working... 4
Plugin 2 working... 4
Plugin 1 working... 5
        Successfully ran plugin: plugin_1
Plugin 2 working... 5
        Successfully ran plugin: plugin_2

Cleaning up plugins...
Plugin 1 cleaned up
        Successfully cleaned up plugin: plugin_1
Plugin 2 cleaned up
        Successfully cleaned up plugin: plugin_2
```

### License
MIT License
