# Oceanlight
My attempt at a cross-platform Vulkan-based renderer.

![Vulkan API](https://img.shields.io/badge/Vulkan-AC162C.svg?style=for-the-badge&logo=vulkan&logoColor=white&logoSize=auto)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![Kubernetes](https://img.shields.io/badge/kubernetes-%23326ce5.svg?style=for-the-badge&logo=kubernetes&logoColor=white)

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
For linux: ```libvulkan-dev``` \
Also on linux make sure you have the glsl compiler installed: ```apt install glslc```

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
