# Bare-metal IREE runtime for Kenning

Copyright (c) 2023 [Antmicro](https://www.antmicro.com)

This repository contains a bare metal implementation of `Runtime` and UART `RuntimeProtocol` for [Kenning](https://github.com/antmicro/kenning), tested with [Renode](https://github.com/antmicro/renode).

The `Runtime` used in this implementation runs models compiled with the [IREE framework](https://github.com/openxla/iree).
The `Runtime` communicates with Kenning via a UART-based `RuntimeProtocol`.
The `Runtime` is tested on the [Springbok](https://opensource.googleblog.com/2022/09/co-simulating-ml-with-springbok-using-renode.html) AI accelerator simulated with Renode.

Apart from bare-metal runtime implementation, this repository provides a demo that:

* Compiles a neural network model (Magic Wand classifier or Person Detection classifier) using Kenning model optimization and deployment framework
* Runs benchmarks on the compiled model in Renode, collecting model quality and performance statistics, including Renode profiling
* Creates a report in MyST format, with interactive and regular PNG plots summarizing the whole execution, which can be either included in Sphinx-based docs or converted to HTML using Jupyter Book

This repository will also demonstrate:

* How to setup Kenning to run Renode and collect profiler information
* How to run Kenning and Renode independently (and how do they communicate)
* How to test the runtime (with Robot framework testing and unit testing)

## Quickstart

This is a minimal set of steps to run base demo scenario.
Follow links in instructions to get an explanation for every step.

First, install `git` and `git-lfs`.
Secondly, clone the repository:

```
git clone --recursive https://github.com/antmicro/kenning-bare-metal-iree-runtime
cd kenning-bare-metal-iree-runtime
git lfs install
git lfs pull
```

The fastest way to obtain all of the necessary dependencies is to [pull the Docker image](#using-the-docker-environment) with all the dependencies for the project:

```
docker pull ghcr.io/antmicro/kenning-bare-metal-iree-runtime:latest
```

and enter it using:

```
docker run --rm -v $(pwd):/data -w /data -it ghcr.io/antmicro/kenning-bare-metal-iree-runtime:latest /bin/bash
```

To build the bare-metal runtime, firstly CMake build needs to be configured.
It can be done with `build_tools/configure_cmake_in_docker.sh`:

```
./build_tools/configure_cmake_in_docker.sh -G Ninja
```

This script will initialize CMake in the `build/build-riscv` directory.
To build the runtime, run:

```
cmake --build build/build-riscv -j `nproc`
```

The runtime binary will be saved in `build/build-riscv/iree-runtime/iree_runtime`.

To evaluate the model, run:

```
python3 -m kenning.scenarios.json_inference_tester kenning-scenarios/renode-magic-wand-iree-bare-metal-inference.json ./results.json
```

For details on what is happening here check [Evaluating the model and accelerator in Renode environment](#evaluating-the-model-and-accelerator-in-simulation).

To render the report with performance and quality metrics (including Renode performance metrics), run:

```
python3 -m kenning.scenarios.render_report \
    --root-dir springbok-magic-wand \
    --report-types performance classification renode_stats \
    --img-dir springbok-magic-wand/img \
    --measurements results.json \
    --model-names magic_wand_fp32 \
    --verbosity INFO \
    v-extensions-riscv springbok-magic-wand/report.md
```

In the end, to create a HTML page from the generated report, run:

```
jupyter-book build springbok-magic-wand/report.md
```

The generated report will be available under `_build/_page/springbok-magic-wand-report/html/index.html`.

The HTML report will contain such information as:

* Scenario used to run the model,
* Inference speed,
* Used instructions during inference,
* Used instructions from V Extensions,
* Memory and interface accesses,
* Model confusion matrix, accuracy, precision, and other quality metrics.

## Building the project

This section describes how to prepare the development environment and build the project.

### Using the Docker environment

The Docker environment with all the necessary components is available in [Dockerfile](./Dockerfile).
The built image can be pulled with:

```
docker pull ghcr.io/antmicro/kenning-bare-metal-iree-runtime:latest
```

### Installing the dependencies in the system

To be able to build the project, several dependencies need to be installed - `cmake`, `git`, `git-lfs`, `ninja-build`, `python3`, `python3-pip`, `wget` and `xxd`.

In Debian-based distros they can be installed with:

```bash
sudo apt-get update
sudo apt-get install -qqy --no-install-recommends \
    cmake \
    g++ \
    git \
    git-lfs \
    ninja-build \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    python3-wheel \
    wget \
    xxd
```

After installation, clone the repository with submodules and go to the created directory:

```
git clone --recursive https://github.com/antmicro/kenning-bare-metal-iree-runtime
cd kenning-bare-metal-iree-runtime
git lfs install
git lfs pull
```

The Python packages required to run below scripts are listed in `requirements.txt`.
Before installing those, it is recommended to create a Python virtual environment.
To create a virtual environment run the following commands:

```bash
python3 -m venv venv                       # create venv
source ./venv/bin/activate                 # activate it
python3 -m pip install -r requirements.txt # install dependencies
```

To install additional python packages required to run Kenning compilation, inference and report rendering, run the following command:

```bash
python3 -m pip install third-party/kenning[tensorflow,reports,uart,renode,object_detection]
```

For installing IREE Python modules for compiling models in Kenning, either follow [Building IREE from source](https://openxla.github.io/iree/building-from-source/#reference-pages) instructions to build Python bindings from `third-party/iree-rv32-springbok/third_party/iree/` directory, or install the available Python modules using `pip`:

```bash
python3 -m pip install iree-compiler~=20230209.425 iree-runtime~=20230209.425 iree-tools-tf~=20230209.425 iree-tools-tflite~=20230209.425
```

> **NOTE:** Downloaded pip package and the binaries downloaded with `third-party/iree-rv32-springbok/build_tools/download_iree_compiler.py` should match to avoid inconsistencies between `iree-compiler` for RISC-V and Python bindings.

### Building the runtime

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

## Evaluating the model and accelerator in simulation

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
                    "iree-llvm-target-cpu-features=+m,+f,+zvl512b,+zve32x,+zve32f",
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

The scenario can be found in `kenning-scenarios/renode-magic-wand-iree-bare-metal-inference.json`.

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
python3 -m kenning.scenarios.json_inference_tester kenning-scenarios/renode-magic-wand-iree-bare-metal-inference.json ./results.json
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

In the end, the report can be converted to HTML using Jupyter Book.
It can be installed with:

```bash
python3 -m pip install install jupyter-book
```

Later, to build the HTML, run:

```bash
jupyter-book build springbok-magic-wand/report.md
```

It uses (_config.yml)[./_config.yml] to configure the build.
It adds usage of `kenning.utils.sphinx_html_as_figure` Sphinx extension to allow using HTML with plots as figures in the rendered report.

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
sudo apt-get install -qqy --no-install-recommends ruby build-essential
sudo gem install ceedling
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
