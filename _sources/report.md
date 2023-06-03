## v-extensions-riscv

### Commands used

````{note}

This section was generated using:

```bash
python -m kenning.scenarios.json_inference_tester \
    kenning-scenarios/renode-magic-wand-iree-bare-metal-inference.json \
    ./results.json

python -m kenning.scenarios.render_report \
    --root-dir \
        springbok-magic-wand \
    --report-types \
        performance \
        classification \
        renode_stats \
    --img-dir \
        springbok-magic-wand/img \
    --measurements \
        results.json \
    --model-names \
        magic_wand_fp32 \
    --verbosity \
        INFO \
        v-extensions-riscv \
        springbok-magic-wand/report.md

```
````

### General information for magic_wand_fp32

*Model framework*:

* tensorflow ver. 2.9.3


*Compiler framework*:


* iree ver. 20230209.425

*Input JSON*:

```json
{
    "dataset": {
        "type": "kenning.datasets.magic_wand_dataset.MagicWandDataset",
        "parameters": {
            "window_size": 128,
            "window_shift": 128,
            "noise_level": 20,
            "dataset_root": "build/MagicWandDataset",
            "inference_batch_size": 1,
            "download_dataset": true,
            "external_calibration_dataset": null,
            "split_fraction_test": 0.2,
            "split_fraction_val": null,
            "split_seed": 1234
        }
    },
    "model_wrapper": {
        "type": "kenning.modelwrappers.classification.tflite_magic_wand.MagicWandModelWrapper",
        "parameters": {
            "model_path": "third-party/kenning/kenning/resources/models/classification/magic_wand.h5",
            "window_size": 128
        }
    },
    "runtime_protocol": {
        "type": "kenning.runtimeprotocols.uart.UARTProtocol",
        "parameters": {
            "port": "/tmp/uart",
            "baudrate": 115200,
            "packet_size": 4096,
            "endianness": "little"
        }
    },
    "runtime": {
        "type": "kenning.runtimes.renode.RenodeRuntime",
        "parameters": {
            "runtime_binary_path": "/home/runner/work/kenning-bare-metal-iree-runtime/kenning-bare-metal-iree-runtime/build/build-riscv/iree-runtime/iree_runtime",
            "platform_resc_path": "/home/runner/work/kenning-bare-metal-iree-runtime/kenning-bare-metal-iree-runtime/sim/config/springbok.resc",
            "profiler_dump_path": null,
            "profiler_interval_step": 10.0,
            "disable_performance_measurements": false
        }
    },
    "optimizers": [
        {
            "type": "kenning.compilers.iree.IREECompiler",
            "parameters": {
                "model_framework": "keras",
                "backend": "llvm-cpu",
                "compiler_args": [
                    "iree-llvm-debug-symbols=false",
                    "iree-vm-bytecode-module-strip-source-map=true",
                    "iree-vm-emit-polyglot-zip=false",
                    "iree-llvm-target-triple=riscv32-pc-linux-elf",
                    "iree-llvm-target-cpu=generic-rv32",
                    "iree-llvm-target-cpu-features=+m,+f,+zvl512b,+zve32x,+zve32f",
                    "iree-llvm-target-abi=ilp32"
                ],
                "compiled_model_path": "build/tflite-magic-wand.vmfb"
            }
        }
    ]
}

```
## Inference performance metrics for magic_wand_fp32


### Inference time

```{figure} img/inference_time.*
---
name: vextensionsriscvmagic_wand_fp32_inferencetime
alt: Inference time
align: center
---

Inference time
```
* *First inference duration* (usually including allocation time): **0.0017979999999999663**,
* *Mean*: **0.0018150785714285808 s**,
* *Standard deviation*: **2.8783196451400254e-05 s**,
* *Median*: **0.0018099999999998673 s**.









## Inference quality metrics for magic_wand_fp32


```{figure} img/confusion_matrix.*
---
name: vextensionsriscvmagic_wand_fp32_confusionmatrix
alt: Confusion matrix
align: center
---

Confusion matrix
```

* *Accuracy*: **1.0**
* *Mean precision*: **0.9999999995982457**
* *Mean sensitivity*: **0.9999999995982457**
* *G-mean*: **0.9999999995982457**
## Renode performance measurements  for magic_wand_fp32


### Count of instructions used during inference

```{figure} img/instr_barplot.*
---
name: vextensionsriscvmagic_wand_fp32_instrbarplot
alt: Count of used instructions during inference
align: center
---

Histogram of used instructions during inference
```
```{figure} img/vector_instr_barplot.*
---
name: vextensionsriscvmagic_wand_fp32_vectorinstrbarplot
alt: Utilization of V Vector Extension instructions during inference
align: center
---

Histogram of V Vector Extension instructions during inference
```
### Executed instructions counters
```{figure} img/executed_instructions_cpu_plot.*
---
name: vextensionsriscvmagic_wand_fp32_cpu_executedinstrplotpath_persecond
alt: Count of executed instructions per second for cpu
align: center
---

Count of executed instructions per second for cpu during benchmark
```

```{figure} img/cumulative_executed_instructions_cpu_plot.*
---
name: vextensionsriscvmagic_wand_fp32_cpu_executedinstrplotpath_cumulative
alt: Cumulative count of executed instructions for cpu
align: center
---

Cumulative count of executed instructions for cpu during benchmark
```
### Memory access counters
```{figure} img/memory_reads_plot.*
---
name: vextensionsriscvmagic_wand_fp32_memoryreadsplotpath_persecond
alt: Count of memory reads per second
align: center
---

Count of memory reads per second during benchmark
```

```{figure} img/cumulative_memory_reads_plot.*
---
name: vextensionsriscvmagic_wand_fp32_memoryreadsplotpath_cumulative
alt: Cumulative count of memory reads
align: center
---

Cumulative count of memory reads during benchmark
```
```{figure} img/memory_writes_plot.*
---
name: vextensionsriscvmagic_wand_fp32_memorywritessplotpath_persecond
alt: Count of memory writes per second
align: center
---

Count of memory writes per second during benchmark
```

```{figure} img/cumulative_memory_writes_plot.*
---
name: vextensionsriscvmagic_wand_fp32_memorywritessplotpath_cumulative
alt: Cumulative count of memory writes
align: center
---

Cumulative count of memory writes during benchmark
```
### Peripheral access counters
```{figure} img/_uart0_reads_plot.*
---
name: vextensionsriscvmagic_wand_fp32_uart0_peripheralreadsplotpath_persecond
alt: Count of uart0 reads per second
align: center
---

Count of uart0 reads per second during benchmark
```

```{figure} img/cumulative_uart0_reads_plot.*
---
name: vextensionsriscvmagic_wand_fp32_uart0_peripheralreadsplotpath_cumulative
alt: Cumulative count of uart0 reads
align: center
---

Cumulative count of uart0 reads during benchmark
```

```{figure} img/_uart0_writes_plot.*
---
name: vextensionsriscvmagic_wand_fp32_uart0_peripheralwritesplotpath_persecond
alt: Count of uart0 writes per second
align: center
---

Count of uart0 writes per second during benchmark
```

```{figure} img/cumulative_uart0_writes_plot.*
---
name: vextensionsriscvmagic_wand_fp32_uart0_peripheralwritesplotpath_cumulative
alt: Cumulative count of uart0 writes
align: center
---

Cumulative count of uart0 writes during benchmark
```
### Instructions stats
* *Instructions counters per inference pass*: **251246**
* *V Vector Extension instructions percentage*: **13.314001754278681** %
* *Top 10 instructions and counters per inference pass*:
    - *addi*: **44372**
    - *lw*: **28960**
    - *sw*: **27796**
    - *bne*: **14350**
    - *bltu*: **13345**
    - *vle32.v*: **13315**
    - *flw*: **12278**
    - *lbu*: **10492**
    - *beq*: **10454**
    - *vfmadd.vf*: **9552**
### Memory allocation stats
* *Host bytes peak*: **1536**
* *Host bytes allocated*: **215040**
* *Host bytes freed*: **213504**
* *Device bytes peak*: **33536**
* *Device bytes allocated*: **2329536**
* *Device bytes freed*: **2312752**
* *Compiled model size*: **26944**

Host memory refers to memory of the CPU controlling the accelerator, while device memory is the memory of the accelerator.
