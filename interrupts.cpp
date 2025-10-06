/**
 * @file interrupts.cpp
 * @author
 *   Adapted for SYSC 4001 Assignment 1 – Interrupt Simulator
 *
 * @brief
 *   Simulates CPU bursts, SYSCALL, and END_IO interrupts based on
 *   a trace file and device/vector tables.
 */

#include "interrupts.hpp"

int main(int argc, char **argv)
{
    // vectors: ISR addresses; delays: device I/O delays
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);
    if (!input_file.is_open())
    {
        std::cerr << "Error: could not open input trace file.\n";
        return 1;
    }

    std::string trace;      //!< current trace line
    std::string execution;  //!< accumulated output text

    /****************** VARIABLES *************************/
    int current_time = 0;      // simulation clock (ms)
    int context_time = 10;     // context save/restore (ms)
    int iret_time = 1;         // IRET time (ms)
    /******************************************************/

    // whitespace cleanup helper
    auto trim = [](std::string &s)
    {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos)
        {
            s.clear();
            return;
        }
        s = s.substr(start, end - start + 1);
    };

    // process each trace line
    while (std::getline(input_file, trace))
    {
        if (trace.empty())
            continue;

        auto [activity, duration_intr] = parse_trace(trace);
        trim(activity);

        // ---------- CASE 1: CPU ----------
        if (activity == "CPU")
        {
            execution += std::to_string(current_time) + ", "
                      + std::to_string(duration_intr) + ", CPU burst\n";
            current_time += duration_intr;
        }

        // ---------- CASE 2: SYSCALL ----------
        else if (activity == "SYSCALL")
        {
            int dev = duration_intr; // device number

            // Steps 1–4  intr_boilerplate helper
            auto [boiler_text, new_time] = intr_boilerplate(current_time, dev, context_time, vectors);
            execution += boiler_text;
            current_time = new_time;

            // Step 5: Execute ISR body 
            int isr_time = delays.at(dev);
            execution += std::to_string(current_time) + ", "
                      + std::to_string(isr_time) + ", execute SYSCALL ISR for device "
                      + std::to_string(dev) + "\n";
            current_time += isr_time;

            // Step 6: IRET
            execution += std::to_string(current_time) + ", "
                      + std::to_string(iret_time) + ", IRET\n";
            current_time += iret_time;
        }

        // ---------- CASE 3: END_IO ----------
        else if (activity == "END_IO")
        {
            int dev = duration_intr; // device number

            // Steps 1–4  intr_boilerplate helper
            auto [boiler_text, new_time] = intr_boilerplate(current_time, dev, context_time, vectors);
            execution += boiler_text;
            current_time = new_time;

            // Step 5: Execute ISR 
            int isr_time = delays.at(dev);
            execution += std::to_string(current_time) + ", "
                      + std::to_string(isr_time) + ", execute END_IO ISR for device "
                      + std::to_string(dev) + "\n";
            current_time += isr_time;

            // Step 6: IRET
            execution += std::to_string(current_time) + ", "
                      + std::to_string(iret_time) + ", IRET\n";
            current_time += iret_time;
        }

        // ---------- CASE 4: UNKNOWN ----------
        else
        {
            execution += "# Warning: unknown activity '" + activity + "'\n";
        }
    }

    input_file.close();

    // write results to output file
    write_output(execution);

    std::cout << "Simulation complete. Output written to execution.txt\n";
    return 0;
}
