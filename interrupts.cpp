/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include <interrupts.hpp>

int main(int argc, char **argv)
{

    // vectors is a C++ std::vector of strings that contain the address of the ISR
    // delays  is a C++ std::vector of ints that contain the delays of each device
    // the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;     //!< string to store single line of trace file
    std::string execution; //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/
    int current_time = 0;                                   // simulation clock (ms)
    int context_save_time = 10;                             // default context save/restore time (ms)
    int isr_activity_time = 40;                             // default ISR body time (ms)
    std::vector<int> pending_completion(delays.size(), -1); // scheduled completion times for devices

    // simple trim lambda to remove leading/trailing whitespace from activity strings
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
    /******************************************************************/
    // parse each line of the input trace file
    while (std::getline(input_file, trace))
    {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        trim(activity);

        if (activity == "CPU")
        {
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;
        }
        else if (activity == "SYSCALL")
        {
            int dev = duration_intr;

            // run interrupt boilerplate (switch to kernel, save context, find vector, load PC)
            auto ret = intr_boilerplate(current_time, dev, context_save_time, vectors);
            execution += ret.first;
            int isr_start = ret.second; // time when ISR body starts

            // execute isr: call device driver
            execution += std::to_string(isr_start) + ", " + std::to_string(isr_activity_time) + ", SYSCALL: run the ISR (device driver)" + " of device " + std::to_string(dev) + "\n";
            int isr_end = isr_start + isr_activity_time;

            // move data from device to memory
            execution += std::to_string(isr_end) + ", " + std::to_string(isr_activity_time) + " transfer data from device to memory\n";
            int data_move = isr_end + isr_activity_time;
            current_time += data_move;

            int isr_time = delays.at(dev);
            execution += std::to_string(current_time) + ", " + std::to_string(isr_time) + ", does calculation" + "\n";
            current_time += isr_time;

            // schedule device completion using device delay from device table
            if (dev >= 0 && dev < (int)pending_completion.size())
            {
                pending_completion[dev] = isr_start + delays.at(dev);
            }
        }
        else if (activity == "END_IO")
        {
            // end of io, use previously scheduled completion time
            int dev = duration_intr;

            // run interrupt boilerplate (switch to kernel, save context, find vector, load PC)
            auto ret = intr_boilerplate(current_time, dev, context_save_time, vectors);
            execution += ret.first;
            int isr_start = ret.second; // time when ISR body starts

            // execute syscall: call commands
            execution += std::to_string(isr_start) + ", " + std::to_string(isr_activity_time) + ", ENDIO: run the syscall interrupt driver\n";
            current_time = isr_start + isr_activity_time;

            execution += std::to_string(current_time) + ", " + std::to_string(delays.at(dev)) + " servicing device starts\n";
            current_time += delays.at(dev);
        }
        else
        {
            // edge case error
            execution += "// Unknown activity: " + activity + "CHECK THISSSSSSS\n";
        }

        /************************************************************************/
    }

    input_file.close();

    write_output(execution);

    return 0;
}
