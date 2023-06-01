# Bare-metal IREE runtime for Kenning

Copyright (c) 2023 [Antmicro](https://www.antmicro.com)

This repository contains a bare metal implementation of `Runtime` and `RuntimeProtocol` for Kenning, tested with Renode.

The `Runtime` used in this implementation runs models compiled with the [IREE framework](https://github.com/openxla/iree).
The `Runtime` communicates with Kenning via a UART-based `RuntimeProtocol`.
The `Runtime` is tested on the [Springbok](https://opensource.googleblog.com/2022/09/co-simulating-ml-with-springbok-using-renode.html) AI accelerator simulated with Renode.

This repository provides an end-to-end demonstration for deploying and testing models in a simulated environment (including the ML accelerator).

## Preparing the environment

To be able to build the project, several dependencies need to be installed.
In Debian-based distros they can be installed with:

```bash
apt-get update
apt-get install -qqy --no-install-recommends \
    cmake \
    git \
    git-lfs \
    ninja-build \
    python3 \
    python3-pip \
    python3-venv \
    wget \
    xxd
```

Then, clone the repository with submodules and go to the created directory:

```
git clone --recursive https://github.com/antmicro/kenning-bare-metal-iree-runtime
cd kenning-bare-metal-iree-runtime
git lfs install
git lfs pull
```

In other distributions install the above dependencies using your package manager.

The Python packages required to run below scripts are listed in `requirements.txt`.
Before installing those, it is recommended to create a Python virtual environment.
To create a virtual environment run the following commands:

```bash
python3 -m venv venv                       # create venv
source ./venv/bin/activate                 # activate it
python3 -m pip install -r requirements.txt # install dependencies
```

To install additional python packages required to run Kenning inference client and render reports, run the following command:

```bash
python3 -m pip install third-party/kenning[tensorflow,reports,uart,renode]
```

For installing IREE Python modules for compiling models in Kenning, either follow [Building IREE from source](https://openxla.github.io/iree/building-from-source/#reference-pages) instructions to build Python bindings from `third-party/iree-rv32-springbok/third_party/iree/` directory, or install the available Python modules using `pip`:

```bash
python3 -m pip install iree-compiler~=20230209.425 iree-runtime~=20230209.425 iree-tools-tf~=20230209.425 iree-tools-tflite~=20230209.425
```

> **NOTE:** Downloaded pip package and the binaries downloaded with `third-party/iree-rv32-springbok/build_tools/download_iree_compiler.py` should match to avoid inconsistencies between `iree-compiler` for RISC-V and Python bindings.

## Building the runtime

Before building the binary, download a pre-compiled RV32 LLVM toolchain and IREE compiler.
To download it, scripts included in `iree-rv32-springbok` submodule located in `third-party/iree-rv32-springbok/build_tools` directory can be used.
To do so, run the following commands in the project's root directory:

```bash
./third-party/iree-rv32-springbok/build_tools/install_toolchain.sh
./third-party/iree-rv32-springbok/build_tools/download_iree_compiler.py
```

> **NOTE:** recent toolchains require at least 2.33 `glibc` version.
In case the `glibc` version in the system is older (this can be checked with `ldd --version`), the older version of the toolchain needs to be downloaded manually from [storage.googleapis.com/shodan-public-artifacts/](https://storage.googleapis.com/shodan-public-artifacts/) (i.e. [toolchain_backups/toolchain_iree_rv32_20220307.tar.gz](https://storage.googleapis.com/shodan-public-artifacts/toolchain_backups/toolchain_iree_rv32_20220307.tar.gz)).

For running the simulation, Renode can be downloaded with:

```bash
./third-party/iree-rv32-springbok/build_tools/download_renode.py
```

To configure CMake, run the `build_tools/configure_cmake.sh` script.
It is possible to add CMake-specific arguments to this script, for example:

```bash
./build_tools/configure_cmake.sh -G Ninja
```

This script will initialize CMake in the `build/build-riscv` directory.

To build the runtime, run:

```bash
cmake --build build/build-riscv -j `nproc`
```

The runtime binary will be saved in `build/build-riscv/iree-runtime` directory.

## Evaluating the model and accelerator in Renode simulation

Kenning can evaluate a bare metal runtime using Renode - it allows the user to:

* Check how the model would behave on actual hardware
* Check the performance and quality of the model on simulated accelerator, to verify any issues with model compilation or accelerator runtime
* Check the instructions used during inference to find out how the accelerator was utilized
* Check the memory and interface accesses during runtime execution

The sample scenario evaluating the model on Springbok AI accelerator in Renode looks as follows:

```json
{
    "dataset": {
        "type": "kenning.datasets.magic_wand_dataset.MagicWandDataset",
        "parameters": {
            "dataset_root": "./build/MagicWandDataset",
            "download_dataset": true
        }
    },
    "model_wrapper": {
        "type": "kenning.modelwrappers.classification.tflite_magic_wand.MagicWandModelWrapper",
        "parameters": {
            "model_path": "third-party/kenning/kenning/resources/models/classification/magic_wand.h5"
        }
    },
    "optimizers":
    [
        {
            "type": "kenning.compilers.iree.IREECompiler",
            "parameters":
            {
                "compiled_model_path": "./build/tflite-magic-wand.vmfb",
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
        "type": "kenning.runtimes.renode.RenodeRuntime",
        "parameters": {
            "runtime_binary_path": "build/build-riscv/iree-runtime/iree_runtime",
            "platform_resc_path": "sim/config/springbok.resc"
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

The scenario can be found in `kenning-scenarios/renode-iree-bare-metal-inference.json`.

The model used for this demo is a simple neural network trained on the Magic Wand dataset for classifying gestures from accelerometer.

* the `dataset` entry provides a class for managing the Magic Wand dataset (downloading the dataset, parsing data from files, and evaluating the model on Springbok by sending inputs and comparing outputs to ground truth)
* the `model_wrapper` entry provides the model to optimize (a TensorFlow implementation of the model in H5 format), as well as model input and output specification, and model-specific preprocessing and postprocessing

The `optimizers` list here contains a single `Optimizer` instance, which is an IREE compiler.
The IREE compiler optimizes and compiles the model to Virtual Machine Flat Buffer format (`.vmfb`), which will be later executed on minimal IREE virtual machine on bare metal in Springbok.
The additional flags are provided to the IREE compiler, specifying the RISC-V target architecture, with V Extensions features to utilize vector computation acceleration present in the Springbok accelerator.

The `runtime` entry specifies the `RenodeRuntime` - it is a runtime that will take:

* `platform_resc_path` - a Renode script (`.resc` file) creating the machine and declaring the UART communication on `/tmp/uart`.
* `runtime_binary_path` - a path to a binary containing the runtime implementation running on platform created in `.resc` file.

The `runtime_protocol` entry specifies the communication parameters.
The UART interface for Renode is created under `/tmp/uart`, hence `port` is set to this path.
In addition, `baudrate` and `endianness` is specified.

Once the scenario is started:

* Kenning loads the model with Keras and compiles it with IREE to `./build/magic-wand.vmfb`
* Kenning runs Renode, creates the machine with accelerator, and requests profiling of the application
* Kenning establishes the connection via UART with the simulated Springbok platform
* Kenning sends the compiled model in `VMFB` format to the device via UART
* The bare-metal IREE runtime loads the model
* Kenning sends the specification of model's inputs and outputs, the bare-metal runtime stores the information
* Kenning sends the input data in loop, the bare metal runtime infers the data and sends results back to Kenning
* Kenning evaluates the model, and Renode profiles the application
* Kenning collects runtime metrics and stores evaluation and benchmark results in `results.json`
* Kenning parses profiling results from Renode and adds them to benchmark results in `results.json`

To evaluate the model, run:

```bash
python3 -m kenning.scenarios.json_inference_tester kenning-scenarios/renode-iree-bare-metal-inference.json ./results.json
```

To render the report with performance and quality metrics (including Renode performance metrics), run:

```bash
python3 -m kenning.scenarios.render_report \
    --root-dir springbok-magic-wand \
    --report-types performance classification renode_stats \
    --img-dir springbok-magic-wand/img \
    --measurements results.json \
    --model-names magic_wand_fp32 \
    --verbosity INFO \
    v-extensions-riscv springbok-magic-wand/report.md
```

This command:

* Creates a `springbok-magic-wand` directory with the report
* Creates plots and summaries regarding overall performance, classification quality and Renode profiler metrics, such as used instructions, memory accesses and more
* Saves plots in `springbok-magic-wand/img` directory
* Creates a MyST-based Markdown file with report summary.

## Running Kenning with existing Renode session.

To run the runtime on Springbok in Renode, use the prepared script as follows:

```
./build_tools/run_simulation.sh
```

It will start the simulation, add Springbok and start the runtime.

In parallel to the simulation in Renode, Kenning can be started with the `third-party/kenning/scripts/jsonconfigs/iree-bare-metal-inference.json` scenario.

It only differs in the `runtime` section:
```json
{
    ...
    "runtime": {
        "type": "kenning.runtimes.iree.IREERuntime",
        "parameters": {
            "driver": "local-task"
        }
    },
    ...
}
```

Instead of using Renode to run the bare metal application, Kenning assumes communication with the actual hardware and communicates via UART.
In `runtime`, the default `IREERuntime` block is used to mark that IREE is used on target device.

```
python3 -m kenning.scenarios.json_inference_tester ./scripts/jsonconfigs/iree-bare-metal-inference.json ./results.json
```

This flow, however, won't collect inference information necessary to plot the performance metrics collected during Renode simulation.

## Running tests

### End-to-end test with Kenning

Kenning and Renode integration can also be verified using a Robot framework test.
Firstly, install the dependencies for tests with:

```bash
python3 -m pip install -r build/renode/tests/requirements.txt
```

To run them you can use a custom `cmake` target `renode-tests` as follows:

```bash
cmake --build build/build-riscv --target renode-tests
```

They can also be executed directly with the following command:

```bash
./build/renode/renode-test renode-tests/test_runtime.robot --show-log -r renode-tests/results
```

### Unit tests

The [Ceedling](https://github.com/ThrowTheSwitch/Ceedling) framework was used for implementing unit tests.
Ruby and Gem are necessary for this framework, for Debian-based distributions you can use:

```bash
apt-get install -qqy --no-install-recommends ruby build-essential
gem install ceedling
```

To run the tests you use custom `cmake` target `unit-tests` as follows:

```bash
cmake --build build/build-riscv --target unit-tests
```

Or run them directly with `ceedling`:

```bash
cd unit-tests
ceedling test:all
```
