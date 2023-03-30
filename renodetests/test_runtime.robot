*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}

*** Variables ***
${UART}                 sysbus.uart0

*** Test Cases ***
Runtime Init Test
    # start the IREE runtime
                        Execute Command                 $bin=@${CURDIR}/../build/build-riscv/iree-runtime/iree_runtime
                        Execute Command                 i @${CURDIR}/../sim/config/springbok.resc
                        Execute Command                 start
                        Execute Command                 sysbus.vec_controlblock WriteDoubleWord 0xc 0
    # crete tester for logs
    ${log_tester}=      Create Log Tester               timeout=5.0
    # verify that runtime started
                        Wait For Log Entry              .*Runtime started.*    treatAsRegex=True

Inference Test
    # start the IREE runtime
                        Execute Command                 $bin=@${CURDIR}/../build/build-riscv/iree-runtime/iree_runtime
                        Execute Command                 i @${CURDIR}/../sim/config/springbok.resc
                        Execute Command                 start
                        Execute Command                 sysbus.vec_controlblock WriteDoubleWord 0xc 0
    # crete testers for logs and UART
    ${log_tester}=      Create Log Tester               timeout=5.0
    ${uart_tester}=     Create Terminal Tester          ${UART}
    # verify that UART is idle
                        Test If Uart Is Idle            timeout=1    testerId=${uart_tester}
    # start Kenning inference client
    ${kenning}=         Start Process                   bash    renodetests/run_kenning_inference_tester.sh   shell=True    cwd=${CURDIR}/..    alias='kenning-process'
                        Sleep                           30s
    # verify that inference client is running and communicating via UART
                        Process Should Be Running       ${kenning}
                        Sleep                           60s
                        Run Keyword And Expect Error    InvalidOperationException:*   Test If Uart Is Idle    timeout=30    testerId=${uart_tester}
                        Process Should Be Running       ${kenning}
                        Sleep                           60s
                        Run Keyword And Expect Error    InvalidOperationException:*   Test If Uart Is Idle    timeout=30    testerId=${uart_tester}
    # verify that there was no runtime errors
                        Should Not Be In Log            .*ERROR.*    treatAsRegex=True
    # verify that inference client is still running
                        Process Should Be Running       ${kenning}
    # kill inference client
                        Terminate Process               ${kenning}
                        Process Should Be Stopped       ${kenning}
                        Sleep                           5s
    # verify that UART is idle
                        Test If Uart Is Idle            timeout=1    testerId=${uart_tester}
    # verify that inference client exited due to kill
    ${kenning_rc}=      Get Process Result              'kenning-process'    rc=True
                        Should Be Equal As Integers     ${kenning_rc}    -15
    # gather opcode stats
    ${opcode_stats}=    Execute Command                 sysbus.cpu GetAllOpcodesCounters
    # remove double newlines
    ${opcode_stats}=    Replace String                  ${opcode_stats}    \n\n    \n
    # print and save stats to file
                        Log To Console                  ${opcode_stats}
                        Create File                     ${CURDIR}/results/opcode_stats.txt  ${opcode_stats}