# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}

*** Variables ***
${UART}                 sysbus.uart0
${RUNTIME_BINARY_PATH}  ${CURDIR}/../build/build-riscv/iree-runtime/iree_runtime

*** Test Cases ***
Runtime Init Test
    [Documentation]     Starts IREE bare-metal runtime and checks if it is initialized correctly
    # start the IREE runtime
                        Execute Command                 $bin=@${RUNTIME_BINARY_PATH}
                        Execute Command                 i @${CURDIR}/../sim/config/springbok.resc
                        Execute Command                 start
                        Execute Command                 sysbus.vec_controlblock WriteDoubleWord 0xc 0
    # crete tester for logs
    ${log_tester}=      Create Log Tester               timeout=5.0
    # verify that runtime started
                        Wait For Log Entry              .*Runtime started.*    treatAsRegex=True

Inference Test
    [Documentation]     Starts IREE bare-metal runtime, performs inference using Kenning and checks if there was no
    ...                 errors
    # start the IREE runtime
                        Log To File                     ${CURDIR}/results/runtime_log.txt
                        ...                             flushAfterEveryWrite=${True}
                        Execute Command                 $bin=@${RUNTIME_BINARY_PATH}
                        Execute Command                 i @${CURDIR}/../sim/config/springbok.resc
                        Execute Command                 start
                        Execute Command                 sysbus.vec_controlblock WriteDoubleWord 0xc 0
    # crete testers for logs and UART
    ${log_tester}=      Create Log Tester               timeout=0.5
    ${uart_tester}=     Create Terminal Tester          ${UART}    timeout=0.5
    # verify that UART is idle
                        Test If Uart Is Idle            timeout=0.5    testerId=${uart_tester}
    # start Kenning inference client
    ${kenning}=         Start Process                   bash    ${CURDIR}/run_kenning_inference_tester.sh
                        ...                             shell=True    cwd=${CURDIR}/..
                        ...                             stdout=${CURDIR}/results/kenning_stdout.txt
                        ...                             stderr=${CURDIR}/results/kenning_stderr.txt
    # verify that inference client is running and communicating via UART
                        Sleep                           5s
                        Process Should Be Running       ${kenning}
                        Run Keyword And Expect Error    InvalidOperationException:*   Test If Uart Is Idle
                        ...                             timeout=30    testerId=${uart_tester}
    # wait for inference to end
    ${result}=          Wait For Process                ${kenning}    timeout=600
    # verify that inference client does not return error
                        Should Be Equal As Integers     ${result.rc}    0
                        Sleep                           1s
    # verify that there were no runtime errors
    ${logs}=            Get File                        ${CURDIR}/results/runtime_log.txt
                        Should Not Contain              ${logs}    ERROR
    # verify that UART is idle
                        Test If Uart Is Idle            timeout=0.5    testerId=${uart_tester}
    # gather opcode stats
    ${opcode_stats}=    Execute Command                 sysbus.cpu GetAllOpcodesCounters
    # remove double newlines
    ${opcode_stats}=    Replace String                  ${opcode_stats}    \n\n    \n
    # print and save stats to file
                        Log To Console                  ${opcode_stats}
                        Create File                     ${CURDIR}/results/opcode_stats.txt  ${opcode_stats}
