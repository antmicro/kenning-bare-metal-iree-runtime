# Bare-metal IREE runtime for Kenning

Copyright (c) 2023 [Antmicro](https://www.antmicro.com)

This is a IREE runtime for Springbok - a RISC-V 32-bit bare-metal platform.
The purpose of this runtime is to deploy ML workloads using [Kenning](https://github.com/antmicro/kenning) framework.

## Preparing the environment

First, clone this repository and make sure that git submodules are updated.
Clone this repository and run:
```bash
git clone https://github.com/antmicro/kenning-bare-metal-iree-runtime.git
cd kenning-bare-metal-iree-runtime
git submodule update --init --recursive
```

To install required system packages run:
```
sudo apt install xxd cmake ninja-build wget
```

The python packages required to run below python scripts and to run Kenning inference tester are listed in the `requirements.txt` file.
Before installing those, it is recommended to create Python virtual environment.
To create virtual environment run the following commands:
```bash
python -m venv venv             # create venv
source ./venv/bin/activate      # activate it
python -m pip install -r ./requirements.txt
```

Finally, add `${HOME}/.local/bin` to your `PATH` variable:

```bash
export PATH=${HOME}/.local/bin:${PATH}
```

## Building the runtime

Before building the binary, you need to download pre-compiled RV32 LLVM toolchain and IREE compiler.
To download it you can use scripts included in `iree-rv32-springbok` submodule located in `third-party/iree-rv32-springbok/build_tools` directory.
To do so, run (you need to call these scripts from root directory of this repository):
```bash
./third-party/iree-rv32-springbok/build_tools/install_toolchain.sh
./third-party/iree-rv32-springbok/build_tools/download_iree_compiler.py
```

> **NOTE:** the most recent toolchain requires at least 2.33 `glibc` version.
In case your `glibc` version is older (can be checked with `ldd --version`), you need to manually download older version of the toolchain from [storage.googleapis.com/shodan-public-artifacts/](https://storage.googleapis.com/shodan-public-artifacts/) (i.e. [toolchain_backups/toolchain_iree_rv32_20220307.tar.gz](https://storage.googleapis.com/shodan-public-artifacts/toolchain_backups/toolchain_backups/toolchain_iree_rv32_20220307.tar.gz)).

You may also want to download portable Renode version to run the runtime in simulation.
You can do it with the script:
```bash
./third-party/iree-rv32-springbok/build_tools/download_renode.py
```

To configure CMake, run the `build_tools/configure_cmake.sh` script.
You can add `cmake` arguments to this script, i.e. you can specify the generator:
```bash
./build_tools/configure_cmake.sh -G Ninja
```
This script will initialize cmake in `build/build-riscv` directory.

To build the runtime, run the following command:
```bash
cmake --build build/build-riscv -j `nproc`
```
The runtime binary will be saved in `build/build-riscv/iree-runtime` directory.

## Running the Renode simulation

To run the runtime on Springbok in Renode, you can use the prepared script as follows:
```
./build_tools/run_simulation.sh
```

It will start simulation, add Springbok and start the runtime.
You can now use Kenning to communicate with it and perform ML model inference.

The UART interface will be created under `/tmp/uart`.

To run Kenning inference client, you can use below scenario:
```json
{
    "dataset": {
        "type": "kenning.datasets.magic_wand_dataset.MagicWandDataset",
        "parameters": {
            "dataset_root": "./build/magic-wand-dataset",
            "download_dataset": true
        }
    },
    "model_wrapper": {
        "type": "kenning.modelwrappers.classification.tflite_magic_wand.MagicWandModelWrapper",
        "parameters": {
            "model_path": "./kenning/resources/models/classification/magic_wand.h5"
        }
    },
    "optimizers":
    [
        {
            "type": "kenning.compilers.iree.IREECompiler",
            "parameters":
            {
                "compiled_model_path": "./build/magic-wand.vmfb",
                "backend": "llvm-cpu",
                "model_framework": "keras",
                "compiler_args": [
                    "iree-llvm-debug-symbols=false",
                    "iree-vm-bytecode-module-strip-source-map=true",
                    "iree-vm-emit-polyglot-zip=false",
                    "iree-llvm-target-triple=riscv32-pc-linux-elf",
                    "iree-llvm-target-cpu=generic-rv32",
                    "iree-llvm-target-cpu-features=+m,+f,+zvl512b,+zve32x",
                    "iree-llvm-target-abi=ilp32"
                ]
            }
        }
    ],
    "runtime": {
        "type": "kenning.runtimes.iree.IREERuntime",
        "parameters": {
            "driver": "local-task"
        }
    },
    "runtime_protocol": {
        "type": "kenning.runtimeprotocols.uart.UARTProtocol",
        "parameters": {
            "port": "/tmp/uart",
            "baudrate": 115200,
            "endianness": "little"
        }
    }
}
```

Here we use MagicWand Keras model that is compiled by IREE compiler with target set to `llvm-cpu`.
The IREE compiler also enables vector instructions supported by Springbok.
Then, the inference tester sends model IO specification (model inputs and outputs shapes and types, model entry function and model name) and compiled model in `vmfb` format to the runtime, which loads it.
As the model is ready, the inference tester can send the input data, request inference and then request inference output.
At the end it also requests runtime statistics which are retrieved by runtime from IREE allocator.

To run this scenario, `cd` into `third-party/kenning` directory, save above scenario in some `json` file (i.e. `iree-bare-metal-inference.json`) and run
```bash
python -m kenning.scenarios.json_inference_tester ./iree-bare-metal-inference.json ./output.json
```

## Running tests

For unit tests implementation, the `ceedling` framework was used.
You may need to install it with the following commands:
```bash
apt-get install ruby
gem install ceedling
```
To run the tests you can use custom `cmake` target `unit-tests` as follows:
```bash
cmake --build build/build-riscv --target unit-tests
```
or run them directly with `ceedling`.
First, `cd` into `unit-tests` directory and then execute `ceedling` as follows:
```bash
cd unit-tests
ceedling test:all
```

There are also created ML model inference tests using `Renode`.
To run them you can use custom `cmake` target `renode-tests` as follows:
```bash
cmake --build build/build-riscv --target renode-tests
```
You can also run them directly with the following command
```bash
build/renode/renode-test renode-tests/test_runtime.robot --show-log -r renode-tests/results
```
