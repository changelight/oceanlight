# Oceanlight


## Getting Started

Follow these instructions to get up and running

### Prerequisites

To use this software you need:

```A GPU-accelerated Vulkan driver``` \
And \
```CMake >= 3.20```

On linux, here are some libraries that you probably already have, but are still required:\
```libgl-dev```, ```libx11-dev```, ```libxrandr-dev```, ```libxinerama-dev```, ```libxcursor-dev```, ```libxi-dev```

### Building
First, ensure you have the latest Vulkan SDK.

For windows: https://www.lunarg.com/vulkan-sdk/ \
For linux: ```libvulkan-dev```

Configure: \
```git clone https://github.com/changelight/oceanlight.git && cd oceanlight && mkdir build && cd build && cmake -G Ninja ..```

(Protip: Use CMake's ```-G``` flag to specify if you want makefiles, .vcxproj files, or ninja. We will use ninja here)

Build:
```ninja```

Run: ```./oceanlight```


## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.


## Authors

* **Red** - *Initial work* - [Red](https://github.com/redrifle)

See also the list of [contributors](https://github.com/changelight/oceanlight/contributors) who participated in this project.

## License

This project is licensed under the GNU General Public License - see the [LICENSE](LICENSE) file for details.
